//
// Created by darren on 03/04/24.
//

#include <cassert>
#include <fstream>
#include <sstream>
#include "PackReader.h"

using namespace Id::Pack;


namespace
{
    template<std::integral T>
    T littleToNative(const T value) requires (std::endian::native != std::endian::little)
    {
        return std::byteswap(value);
    }

    template<std::integral T>
    T littleToNative(const T value) requires (std::endian::native == std::endian::little)
    {
        return value;
    }
}


PackReader::File::File(std::istream &in, std::istream::pos_type offset, std::streamsize size) noexcept
: m_inStream(in),
  m_offset(offset),
  m_size(size)
{}


void PackReader::File::seek(int pos) noexcept
{
    assert(0 <= pos && pos < size());
    m_readPos = pos;
}


std::string PackReader::File::read(int bytes)
{
    // stream is shared, other clients may have read so we have to reposition read cursor
    m_inStream.seekg(m_readPos);
    std::string ret(bytes, 0);
    m_inStream.read(ret.data(), bytes);

    if (m_inStream.fail()) {
        throw std::runtime_error("Error reading data for file");
    }

    m_readPos = m_inStream.tellg() - m_offset;
    return ret;
}


std::string PackReader::File::contents() const noexcept
{
    m_inStream.seekg(m_readPos);
    std::string ret(size(), 0);
    m_inStream.read(ret.data(), size());
    return ret;
}


std::ostream & operator<<(std::ostream & out, const PackReader::File & file) noexcept
{
    out << static_cast<std::string>(file);
    return out;
}


PackReader::PackReader(std::istream & in)
: PackReader(&in, false)
{}


PackReader::PackReader(const std::string & fileName)
: PackReader(new std::ifstream(fileName), true)
{}


PackReader::PackReader(std::istream * in, bool owned)
: m_inStream(in),
  m_ownedStream(owned),
  m_header{}
{
    assert(nullptr != in);
    in->seekg(0);
    in->read(m_header.id, 4);

    // "PACK"
    if (0x5041434b != *reinterpret_cast<std::uint32_t *>(m_header.id)) {
        throw std::runtime_error((std::ostringstream() << "Header identifier incorrect - expected \"PACK\" found \"" << m_header.id << "\"").str());
    }

    *in >> m_header.indexOffset;
    *in >> m_header.indexSize;

    m_header.indexOffset = littleToNative(m_header.indexOffset);
    m_header.indexSize = littleToNative(m_header.indexSize);
}


PackReader::~PackReader() noexcept
{
    if (m_ownedStream) {
        delete m_inStream;
    }

    m_inStream = nullptr;
}


void PackReader::ensureIndex() const noexcept
{
    if (0 == m_fileIndex.size()) {
        m_fileIndex.clear();
        m_fileIndexByName.clear();
        m_inStream->seekg(m_header.indexOffset);

        for (int idx = 0; idx < fileCount(); ++idx) {
            IndexEntry entry;
            m_inStream->read(entry.fileName, 56);
            *m_inStream >> entry.fileOffset;
            *m_inStream >> entry.fileSize;
            entry.index = idx;

            entry.fileOffset = littleToNative(entry.fileOffset);
            entry.fileSize = littleToNative(entry.fileSize);

            m_fileIndexByName[entry.fileName] = entry;
            m_fileIndex.push_back(std::move(entry));
        }
    }
}


int PackReader::fileCount() const noexcept
{
    return m_header.indexSize / sizeof(IndexEntry);
}


bool PackReader::has(const std::string & fileName) const noexcept
{
    ensureIndex();
    return m_fileIndexByName.contains(fileName);
}


std::string PackReader::fileName(int idx) const noexcept
{
    assert(0 <= idx && fileCount() > idx);
    ensureIndex();
    return m_fileIndex[idx].fileName;
}


std::optional<int> PackReader::fileIndex(const std::string & fileName) const noexcept
{
    ensureIndex();

    if (!m_fileIndexByName.contains(fileName)) {
        return {};
    }

    return m_fileIndexByName[fileName].index;
}


PackReader::File PackReader::file(int idx) const noexcept
{
    assert(0 <= idx && fileCount() > idx);
    ensureIndex();
    return File(*m_inStream, m_fileIndex[idx].fileOffset, m_fileIndex[idx].fileSize);
}


PackReader::File PackReader::file(const std::string & fileName) const noexcept
{
    ensureIndex();
    assert(has(fileName));
    const auto & indexEntry = m_fileIndexByName[fileName];
    return File(*m_inStream, indexEntry.fileOffset, indexEntry.fileSize);
}


void PackReader::extract(int idx, std::ostream & out) const
{
    out << file(idx);
}


void PackReader::extract(const std::string & fileName, std::ostream & out) const
{
    out << file(fileName);
}


void PackReader::extract(int idx, const std::string & outputFile) const
{
    auto out = std::ofstream(outputFile);
    extract(idx, out);
}


void PackReader::extract(const std::string & fileName, const std::string & outputFile) const
{
    auto out = std::ofstream(outputFile);
    extract(fileName, out);
}

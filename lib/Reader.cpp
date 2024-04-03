//
// Created by darren on 03/04/24.
//

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include "Reader.h"

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

    std::uint32_t readUint32(std::istream & in)
    {
        static char bytes[4];
        in.read(bytes, 4);
        return littleToNative(*(reinterpret_cast<std::uint32_t *>(bytes)));
    }
}


Reader::File::File(std::istream &in, std::istream::pos_type offset, std::streamsize size) noexcept
: m_inStream(in),
  m_offset(offset),
  m_size(size)
{}


void Reader::File::seek(int pos) noexcept
{
    assert(0 <= pos && pos < size());
    m_readPos = pos;
}


std::string Reader::File::read(int bytes)
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


std::string Reader::File::contents() const noexcept
{
    m_inStream.seekg(m_readPos);
    std::string ret(size(), 0);
    m_inStream.read(ret.data(), size());
    return ret;
}


std::ostream & Id::Pack::operator<<(std::ostream & out, const Reader::File & file) noexcept
{
    out << static_cast<std::string>(file);
    return out;
}


const Reader::Iterator Reader::Iterator::operator++(int)
{
    Iterator ret(*this);

    if (m_index < m_reader.fileCount()) {
        ++m_index;
    }

    return ret;
}


Reader::Iterator & Reader::Iterator::operator++()
{
    if (m_index < m_reader.fileCount()) {
        ++m_index;
    }

    return *this;
}


const Reader::File Reader::Iterator::operator*() const
{
    return m_reader.file(m_index);
}


Reader::File Reader::Iterator::operator*()
{
    return m_reader.file(m_index);
}

const Reader::File Reader::Iterator::operator->() const
{
    return m_reader.file(m_index);
}


Reader::File Reader::Iterator::operator->()
{
    return m_reader.file(m_index);
}


Reader::Reader(std::istream & in)
: Reader(&in, false)
{}


Reader::Reader(const std::string & fileName)
: Reader(new std::ifstream(fileName), true)
{}


Reader::Reader(std::istream * in, bool owned)
: m_inStream(in),
  m_ownedStream(owned),
  m_header{}
{
    assert(nullptr != in);
    in->seekg(0);
    in->read(m_header.id, 4);

    if (std::string("PACK") != m_header.id) {
        throw std::runtime_error((std::ostringstream() << "Header identifier incorrect - expected \"PACK\" found \"" << m_header.id << "\"").str());
    }

    m_header.indexOffset = readUint32(*in);
    m_header.indexSize = readUint32(*in);
}


Reader::~Reader() noexcept
{
    if (m_ownedStream) {
        delete m_inStream;
    }

    m_inStream = nullptr;
}


void Reader::ensureIndex() const noexcept
{
    if (0 == m_fileIndex.size()) {
        m_fileIndex.clear();
        m_fileIndexByName.clear();
        m_inStream->seekg(m_header.indexOffset);

        for (int idx = 0; idx < fileCount(); ++idx) {
            IndexEntry entry;
            m_inStream->read(entry.fileName, 56);
            entry.index = idx;

            entry.fileOffset = readUint32(*m_inStream);
            entry.fileSize = readUint32(*m_inStream);

            m_fileIndexByName[entry.fileName] = entry;
            m_fileIndex.push_back(entry);
        }
    }
}


int Reader::fileCount() const noexcept
{
    return m_header.indexSize / sizeof(IndexEntry);
}


bool Reader::has(const std::string & fileName) const noexcept
{
    ensureIndex();
    return m_fileIndexByName.contains(fileName);
}


std::string Reader::fileName(int idx) const noexcept
{
    assert(0 <= idx && fileCount() > idx);
    ensureIndex();
    return m_fileIndex[idx].fileName;
}


int Reader::fileIndex(const std::string & fileName) const noexcept
{
    ensureIndex();
    assert(has(fileName));
    return m_fileIndexByName[fileName].index;
}


int Reader::fileOffset(int idx) const noexcept
{
    assert(0 <= idx && fileCount() > idx);
    ensureIndex();
    return static_cast<int>(m_fileIndex[idx].fileOffset);
}


int Reader::fileOffset(const std::string & fileName) const noexcept
{
    ensureIndex();
    assert(has(fileName));
    return static_cast<int>(m_fileIndexByName[fileName].fileOffset);
}

int Reader::fileSize(int idx) const noexcept
{
    assert(0 <= idx && fileCount() > idx);
    ensureIndex();
    return static_cast<int>(m_fileIndex[idx].fileSize);
}


int Reader::fileSize(const std::string & fileName) const noexcept
{
    ensureIndex();
    assert(has(fileName));
    return static_cast<int>(m_fileIndexByName[fileName].fileSize);
}


Reader::File Reader::file(int idx) const noexcept
{
    assert(0 <= idx && fileCount() > idx);
    ensureIndex();
    return File(*m_inStream, m_fileIndex[idx].fileOffset, m_fileIndex[idx].fileSize);
}


Reader::File Reader::file(const std::string & fileName) const noexcept
{
    ensureIndex();
    assert(has(fileName));
    const auto & indexEntry = m_fileIndexByName[fileName];
    return File(*m_inStream, indexEntry.fileOffset, indexEntry.fileSize);
}


void Reader::extract(int idx, std::ostream & out) const
{
    out << file(idx);
}


void Reader::extract(const std::string & fileName, std::ostream & out) const
{
    out << file(fileName);
}


void Reader::extract(int idx, const std::string & outputFile) const
{
    auto out = std::ofstream(outputFile);
    extract(idx, out);
}


void Reader::extract(const std::string & fileName, const std::string & outputFile) const
{
    auto out = std::ofstream(outputFile);
    extract(fileName, out);
}


Reader::Iterator Reader::begin()
{
    return Iterator(*this, 0);
}


Reader::Iterator Reader::end()
{
    return Iterator(*this, fileCount());
}

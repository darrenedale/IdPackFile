//
// Created by darren on 03/04/24.
//

#ifndef LIBPACKFILE_PACKREADER_H
#define LIBPACKFILE_PACKREADER_H

#include <cstdint>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace Id::Pack
{
    class Reader
    {
    public:
        class File
        {
        friend class Reader;

        public:
            int size() const noexcept
            {
                return static_cast<int>(m_size);
            }

            int location() const noexcept
            {
                return static_cast<int>(m_offset);
            }

            int pos() const noexcept
            {
                return static_cast<int>(m_readPos);
            }

            bool eof() const noexcept
            {
                return m_readPos >= m_size;
            }

            void reset() noexcept
            {
                m_readPos = 0;
            }

            void seek(int pos) noexcept;

            std::string read(int bytes);

            std::string contents() const noexcept;

            explicit operator std::string() const noexcept
            {
                return contents();
            }

        private:
            File(std::istream & in, std::istream::pos_type offset, std::streamsize size) noexcept;

            std::istream & m_inStream;
            std::istream::pos_type m_offset;
            std::streamsize m_size;
            std::istream::pos_type m_readPos = 0;
        };

        class Iterator final
        {
            friend class Reader;

        public:
            Iterator(const Iterator &) = default;
            Iterator(Iterator &&) = default;
            Iterator & operator=(const Iterator &) = delete;
            Iterator & operator=(Iterator &&) = delete;

            const Iterator operator++(int);
            Iterator & operator++();
            const File operator*() const;
            File operator*();
            const File operator->() const;
            File operator->();

            bool operator==(const Iterator & other) const
            {
                return &(other.m_reader) == &m_reader && other.m_index == m_index;
            }

        private:
            explicit Iterator(Reader & reader, int index)
                    : m_reader(reader),
                      m_index(index)
            {}

            Reader & m_reader;
            int m_index;
        };

        explicit Reader(const std::string & fileName);

        /**
         * @param stream The stream to read from. The caller is responsible for ensuring the stream lives as long
         * as the reader using it (and any files it yields).
         */
        explicit Reader(std::istream & stream);
        Reader(const Reader &) = delete;
        Reader(Reader &&) = delete;
        void operator = (const Reader &) = delete;
        void operator = (Reader &&) = delete;
        virtual ~Reader() noexcept;

        int fileCount() const noexcept;

        bool has(const std::string & fileName) const noexcept;

        std::string fileName(int idx) const noexcept;
        int fileIndex(const std::string & fileName) const noexcept;

        int fileOffset(int idx) const noexcept;
        int fileOffset(const std::string & fileName) const noexcept;

        int fileSize(int idx) const noexcept;
        int fileSize(const std::string & fileName) const noexcept;

        File file(int idx) const noexcept;
        File file(const std::string & name) const noexcept;

        void extract(int idx, const std::string & outputFile) const;
        void extract(const std::string & fileName, const std::string & outputFile) const;

        void extract(int idx, std::ostream & out) const;
        void extract(const std::string & fileName, std::ostream & out) const;

        Iterator begin();
        Iterator end();

    private:
        struct Header
        {
            char id[4];
            std::uint32_t indexOffset;
            std::uint32_t indexSize;
        };

        struct IndexEntry
        {
            char fileName[56];
            std::uint32_t fileOffset;
            std::uint32_t fileSize;
            int index;
        };

        Reader(std::istream * in, bool owned);

        void ensureIndex() const noexcept;

        std::istream * m_inStream;
        bool m_ownedStream;
        Header m_header;

        mutable std::map<std::string, IndexEntry> m_fileIndexByName;
        mutable std::vector<IndexEntry> m_fileIndex;
    };

    std::ostream & operator<<(std::ostream & out, const Reader::File & file) noexcept;
}

#endif

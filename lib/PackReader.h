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

namespace Id
{
    namespace Pack
    {
        class PackReader
        {
        public:
            class File
            {
            friend class PackReader;

            public:
                int size() const noexcept
                {
                    return static_cast<int>(m_size);
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

            explicit PackReader(const std::string & fileName);

            /**
             * @param stream The stream to read from. The caller is responsible for ensuring the stream lives as long
             * as the reader using it (and any files it yields).
             */
            explicit PackReader(std::istream & stream);
            PackReader(const PackReader &) = delete;
            PackReader(PackReader &&) = delete;
            void operator = (const PackReader &) = delete;
            void operator = (PackReader &&) = delete;
            virtual ~PackReader() noexcept;

            int fileCount() const noexcept;

            bool has(const std::string & fileName) const noexcept;

            std::string fileName(int idx) const noexcept;
            std::optional<int> fileIndex(const std::string & fileName) const noexcept;

            File file(int idx) const noexcept;
            File file(const std::string & name) const noexcept;

            void extract(int idx, const std::string & outputFile) const;
            void extract(const std::string & fileName, const std::string & outputFile) const;

            void extract(int idx, std::ostream & out) const;
            void extract(const std::string & fileName, std::ostream & out) const;

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

            PackReader(std::istream * in, bool owned);

            void ensureIndex() const noexcept;

            std::istream * m_inStream;
            bool m_ownedStream;
            Header m_header;

            mutable std::map<std::string, IndexEntry> m_fileIndexByName;
            mutable std::vector<IndexEntry> m_fileIndex;
        };

        std::ostream operator<<(std::ostream & out, const PackReader::File & file) noexcept;
    }
}

#endif

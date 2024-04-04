#ifndef LIBIDPAK_PACKREADER_H
#define LIBIDPAK_PACKREADER_H

#include <cstdint>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace Id::Pack
{
    /**
     * Reads ID PACK archives (.pak).
     */
    class Reader
    {
    public:
        /**
         * A thin wrapper around the PACK file stream for a single file in the archive.
         *
         * Think of this as a sort of std::span for the portion of the PACK archive that contains a single file.
         */
        class File
        {
        // the reader is a friend so that it can construct File instances
        friend class Reader;

        public:
            /** @return The size, in bytes, of the file. */
            int size() const noexcept
            {
                return static_cast<int>(m_size);
            }

            /** @return The offset from the start of the file from which the next byte will be read. */
            int pos() const noexcept
            {
                return static_cast<int>(m_readPos);
            }

            /** @return Whether random access reading has progressed beyond the end of the file. */
            bool eof() const noexcept
            {
                return m_readPos >= m_size;
            }

            /** Reset the read position to the start of the file. */
            void reset() noexcept
            {
                m_readPos = 0;
            }

            /** Seek to a given byte offset in the file. */
            void seek(int pos) noexcept;

            /** Read a number of bytes from the file, starting at the current read position. */
            std::string read(int bytes);

            /**
             * Read all the content of the file.
             *
             * The current read position is unaffected by this call - fetching the full content is entirely isolated
             * from random-access reading.
             */
            std::string contents() const noexcept;

            /**
             * Cast the File to a string.
             *
             * The string represents the full, unmodified content of the file.
             *
             * The current read position is unaffected by this call - casting to a string is entirely isolated from
             * random-access reading.
             */
            explicit operator std::string() const noexcept
            {
                return contents();
            }

        private:
            // there's no public constructor, only Reader objects can instantiate Files
            File(std::istream & in, std::istream::pos_type offset, std::streamsize size) noexcept;

            /** The stream for the PACK archive that contains the file. */
            std::istream & m_inStream;

            /** The byte offset in the stream where the file starts. */
            std::istream::pos_type m_offset;

            /** The sie in bytes of the file. */
            std::streamsize m_size;

            /** For random access, the current read position (relative to the offset of the start of the file). */
            std::istream::pos_type m_readPos = 0;
        };

        /**
         * An iterator class so that the files in a PACK archive can be iterated using STL algorithms and range-for
         * loops.
         */
        class Iterator final
        {
        // the reader is a friend so that it can construct Iterator instances
        friend class Reader;

        public:
            /** Copy an iterator. */
            Iterator(const Iterator &) = default;

            /** Move an iterator. */
            Iterator(Iterator &&) = default;

            // iterators can't be copy or move assigned as they have reference members
            Iterator & operator=(const Iterator &) = delete;
            Iterator & operator=(Iterator &&) = delete;

            /**
             * Post-increment the iterator to the next file.
             *
             * @return A copied iterator pointing to the file the original iterator was pointing to before it was
             * incremented.
             */
            const Iterator operator++(int);

            /**
             * Pre-increment the iterator to the next file.
             *
             * @return A reference to the iterator, now pointing to the next file.
             */
            Iterator & operator++();

            /** Dereference the iterator to retrieve a const copy of the File it points to. */
            const File operator*() const;

            /** Dereference the iterator to retrieve a copy of the File it points to. */
            File operator*();

            /** Perform indirection on the iterator to access a member of the (const) File it points to. */
            const File operator->() const;

            /** Perform indirection on the iterator to access a member of the File it points to. */
            File operator->();

            /**
             * Check for equality between two iterators.
             *
             * Two iterators are equal if they reference the same underlying Reader instance and point to the same file.
             */
            bool operator==(const Iterator & other) const
            {
                return &(other.m_reader) == &m_reader && other.m_index == m_index;
            }

        private:
            // there is no public constructor other than the copy and move constructors - only Reader instances can
            // create new iterators
            explicit Iterator(Reader & reader, int index)
            : m_reader(reader),
              m_index(index)
            {}

            /** The Reader whose files are being iterated. */
            Reader & m_reader;

            /**
             * The 0-based index of the file the iterator points to.
             *
             * This will equal the number of files in the Reader when the iterator has passed the end of the set of
             * files.
             */
            int m_index;
        };

        /**
         * Initialise a new Reader to read a PACK archive from a file.
         *
         * @param fileName The file to read.
         */
        explicit Reader(const std::string & fileName);

        /**
         * @param stream The stream to read from. The caller is responsible for ensuring the stream lives as long
         * as the reader using it (and any files it yields).
         */
        explicit Reader(std::istream & stream);

        // Reader instances can't be copied or moved
        Reader(const Reader &) = delete;
        Reader(Reader &&) = delete;
        void operator = (const Reader &) = delete;
        void operator = (Reader &&) = delete;
        virtual ~Reader() noexcept;

        /** @return The number of files in the PACK archive. */
        int fileCount() const noexcept;

        /**
         * Check whether a named file exists in the archive.
         *
         * The name matching is very strict - it's case-sensitive, does not allow for leading / separators if the
         * archive file name doesn't have them, does not resolve . or .., and, and does not collapse sequences of /
         * separators.
         *
         * @param fileName The name of the file to look for.
         */
        bool has(const std::string & fileName) const noexcept;

        /**
         * Look up the name of a file from its position in the archive.
         *
         * The provided index must be >= 0 and < fileCount().
         *
         * @param idx The 0-based index of the file.
         *
         * @return The name of the file.
         */
        std::string fileName(int idx) const noexcept;

        /**
         * Look up the index of a named file in the archive.
         *
         * The provided file name must be in the archive, as determined by has().
         *
         * @param fileName The file to look for.
         *
         * @return The index of the file.
         */
        int fileIndex(const std::string & fileName) const noexcept;

        /**
         * Look up the byte offset of a file in the archive.
         *
         * The provided index must be >= 0 and < fileCount().
         *
         * @param idx The 0-based index of the file.
         *
         * @return The byte offset of the file inside the PACK archive.
         */
        int fileOffset(int idx) const noexcept;

        /**
         * Look up the byte offset of a file in the archive.
         *
         * The provided file name must be in the archive, as determined by has().
         *
         * @param fileName The file to look for.
         *
         * @return The byte offset of the file inside the PACK archive.
         */
        int fileOffset(const std::string & fileName) const noexcept;

        /**
         * Look up the byte size of a file in the archive.
         *
         * The provided index must be >= 0 and < fileCount().
         *
         * @param idx The 0-based index of the file.
         *
         * @return The byte size of the file inside the PACK archive.
         */
        int fileSize(int idx) const noexcept;

        /**
         * Look up the byte size of a file in the archive.
         *
         * The provided file name must be in the archive, as determined by has().
         *
         * @param fileName The file to look for.
         *
         * @return The byte size of the file inside the PACK archive.
         */
        int fileSize(const std::string & fileName) const noexcept;

        /**
         * Get a file from the archive.
         *
         * The provided index must be >= 0 and < fileCount().
         *
         * @param idx The 0-based index of the file.
         *
         * @return A thin wrapper around the chunk of the archive that contains the file's content.
         */
        File file(int idx) const noexcept;

        /**
         * Get a file from the archive.
         *
         * The provided file name must be in the archive, as determined by has().
         *
         * @param fileName The file to look for.
         *
         * @return A thin wrapper around the chunk of the archive that contains the file's content.
         */
        File file(const std::string & name) const noexcept;

        /**
         * Extract a file from the archive to a file in the local filesystem.
         *
         * The provided index must be >= 0 and < fileCount().
         *
         * @param idx The 0-based index of the file.
         * @param outputFile The path to which to save the extracted file locally.
         */
        void extract(int idx, const std::string & outputFile) const;

        /**
         * Extract a file from the archive to a file in the local filesystem.
         *
         * The provided file name must be in the archive, as determined by has().
         *
         * @param fileName The file to extract.
         * @param outputFile The path to which to save the extracted file locally.
         */
        void extract(const std::string & fileName, const std::string & outputFile) const;

        /**
         * Extract a file from the archive and write its content to a stream.
         *
         * The provided index must be >= 0 and < fileCount().
         *
         * @param idx The 0-based index of the file.
         * @param out The stream to which to write the extracted file content.
         */
        void extract(int idx, std::ostream & out) const;

        /**
         * Extract a file from the archive and write its content to a stream.
         *
         * The provided file name must be in the archive, as determined by has().
         *
         * @param fileName The file to extract.
         * @param out The stream to which to write the extracted file content.
         */
        void extract(const std::string & fileName, std::ostream & out) const;

        /** @return an Iterator pointing to the first file in the archive. */
        Iterator begin();

        /** @return an Iterator pointing past the last file in the archive. */
        Iterator end();

    private:
        /**
         * The structure of the PACK archive header.
         */
        struct Header
        {
            char id[4];
            std::uint32_t indexOffset;
            std::uint32_t indexSize;
        };

        /**
         * The structure of the entry in the PACK archive's index for a single file (augmented with the 0-based index of
         * the file within the archive, for use internally.
         */
        struct IndexEntry
        {
            char fileName[56];
            std::uint32_t fileOffset;
            std::uint32_t fileSize;
            int index;
        };

        /**
         * Internal constructor to which all other constructors delegate.
         *
         * @param in The stream from which the archive is being read.
         * @param owned Whether or not the stream pointer is owned by the Reader instance.
         */
        Reader(std::istream * in, bool owned);

        /** Lazy-load the file index from the PACK archive. */
        void ensureIndex() const noexcept;

        /** The input stream from which the archive is being read. */
        std::istream * m_inStream;

        /** Whether the Reader object owns the stream pointer, and will delete it on destruction. */
        bool m_ownedStream;

        /** The header read from the PACK archive stream. */
        Header m_header;

        // The file indices, lazy-loaded on-demanded by ensureIndex()
        mutable std::map<std::string, IndexEntry> m_fileIndexByName;
        mutable std::vector<IndexEntry> m_fileIndex;
    };

    /** Output a File from a PACK archive to an output stream. */
    std::ostream & operator<<(std::ostream & out, const Reader::File & file) noexcept;
}

#endif

#include "project_archive.h"
#include <archive.h>
#include <archive_entry.h>
#include <filesystem>
#include <fstream>
#include <vector> // Added for std::vector

namespace disgrace_ns
{

    bool disgrace_ns::ProjectArchive::save(const ::std::string& path,
                              const ::std::string& folder)
    {
        struct archive* a = archive_write_new();
        archive_write_add_filter_gzip(a);
        archive_write_set_format_pax_restricted(a);

        if (archive_write_open_filename(a, path.c_str()) != ARCHIVE_OK)
            return false;

        for (auto& p :
            ::std::filesystem::recursive_directory_iterator(folder))
        {
            if (!p.is_regular_file())
                continue;

            struct archive_entry* entry =
            archive_entry_new();

            archive_entry_set_pathname(
                entry,
                ::std::filesystem::relative(
                    p.path(), folder).string().c_str());

            archive_entry_set_size(
                entry,
                ::std::filesystem::file_size(p.path()));

            archive_entry_set_filetype(entry, AE_IFREG);
            archive_entry_set_perm(entry, 0644);

            archive_write_header(a, entry);

            ::std::ifstream file(p.path(),
                               ::std::ios::binary);

            ::std::vector<char> buffer(
                ::std::filesystem::file_size(p.path()));

            file.read(buffer.data(), buffer.size());

            archive_write_data(a,
                               buffer.data(),
                               buffer.size());

            archive_entry_free(entry);
        }

        archive_write_close(a);
        archive_write_free(a);

        return true;
    }

    bool disgrace_ns::ProjectArchive::extract(const ::std::string& path,
                                 const ::std::string& dest)
    {
        struct archive* a = archive_read_new();
        archive_read_support_format_tar(a);
        archive_read_support_filter_gzip(a);

        if (archive_read_open_filename(a,
            path.c_str(),
                                       10240) != ARCHIVE_OK)
            return false;

        struct archive_entry* entry;

        while (archive_read_next_header(a, &entry)
            == ARCHIVE_OK)
        {
            ::std::filesystem::path out =
            ::std::filesystem::path(dest) /
            archive_entry_pathname(entry);

            ::std::filesystem::create_directories(
                out.parent_path());

            ::std::ofstream file(out,
                               ::std::ios::binary);

            const void* buff;
            size_t size;
            la_int64_t offset;

            while (archive_read_data_block(a,
                &buff,
                &size,
                &offset)
                == ARCHIVE_OK)
            {
                file.write((const char*)buff, size);
            }
        }

        archive_read_close(a);
        archive_read_free(a);

        return true;
    }

} // namespace disgrace_ns

bool read_file_using_memory_map()
{
    // current directory 를 구한다.
    wchar_t *buf = NULL;
    uint32_t buflen = 0;
    buflen = GetCurrentDirectoryW(buflen, buf);
    if (0 == buflen)
    {
        print("err ] GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
        return false;
    }

    buf = (PWSTR)malloc(sizeof(WCHAR) * buflen);
    if (0 == GetCurrentDirectoryW(buflen, buf))
    {
        print("err ] GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
        free(buf);
        return false;
    }

    // current dir \\ test.txt 파일명 생성
    wchar_t file_name[260];
    if (!SUCCEEDED(StringCbPrintfW(
        file_name,
        sizeof(file_name),
        L"%ws\\test.txt",
        buf)))
    {
        print("err ] can not create file name");
        free(buf);
        return false;
    }
    free(buf); buf = NULL;

    if (true != is_file_existsW(file_name))
    {
        print("err ] no file exists. file = %ws", file_name);
        return false;
    }

    HANDLE file_handle = CreateFileW(
        (LPCWSTR)file_name,
        GENERIC_READ,
        NULL,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );
    if (INVALID_HANDLE_VALUE == file_handle)
    {
        print("err ] CreateFile(%ws) failed, gle = %u", file_name, GetLastError());
        return false;
    }

    // check file size
    //
    LARGE_INTEGER fileSize;
    if (TRUE != GetFileSizeEx(file_handle, &fileSize))
    {
        print("err ] GetFileSizeEx(%ws) failed, gle = %u", file_name, GetLastError());
        CloseHandle(file_handle);
        return false;
    }

    // [ WARN ]
    //
    // 4Gb 이상의 파일의 경우 MapViewOfFile()에서 오류가 나거나
    // 파일 포인터 이동이 문제가 됨
    // FilIoHelperClass 모듈을 이용해야 함
    //
    _ASSERTE(fileSize.HighPart == 0);
    if (fileSize.HighPart > 0)
    {
        print("file size = %I64d (over 4GB) can not handle. use FileIoHelperClass",
            fileSize.QuadPart);
        CloseHandle(file_handle);
        return false;
    }

    DWORD file_size = (DWORD)fileSize.QuadPart;
    HANDLE file_map = CreateFileMapping(
        file_handle,
        NULL,
        PAGE_READONLY,
        0,
        0,
        NULL
        );
    if (NULL == file_map)
    {
        print("err ] CreateFileMapping(%ws) failed, gle = %u", file_name, GetLastError());
        CloseHandle(file_handle);
        return false;
    }

    PCHAR file_view = (PCHAR)MapViewOfFile(
        file_map,
        FILE_MAP_READ,
        0,
        0,
        0
        );
    if (file_view == NULL)
    {
        print("err ] MapViewOfFile(%ws) failed, gle = %u", file_name, GetLastError());

        CloseHandle(file_map);
        CloseHandle(file_handle);
        return false;
    }

    // do some io
    char a = file_view[0];  // 0x d9
    char b = file_view[1];  // 0xb3



                            // close all
    UnmapViewOfFile(file_view);
    CloseHandle(file_map);
    CloseHandle(file_handle);
    return true;
}

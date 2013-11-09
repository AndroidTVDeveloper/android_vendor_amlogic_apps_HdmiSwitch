#ifndef AMSYSWRITE_UTILS_H
#define AMSYSWRITE_UTILS_H

#ifdef  __cplusplus
extern "C" {
#endif
    int systemWriteReadSysfs(const char* path, char* value);
    int systemWriteReadNumSysfs(const char* path, char* value, int size);
    int systemWriteWriteSysfs(const char* path, char* value);

#ifdef  __cplusplus
}
#endif


#endif

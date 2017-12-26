#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#include <sys/cygwin.h>
//#include <share.h>
/*FILE *_fdopen(    
   int fd,  
   const char *mode   
);
int _open_osfhandle (  
   intptr_t osfhandle,  
   int flags   
);*/

#define sector_size  (2048U)

struct chunk{
	unsigned offset;
	FILE* fp;
};

static char buff[4096][sector_size];

static const char __check[((sizeof(buff[0]) == sector_size)) - 1];

int main(int argc, char** argv){
	assert(!(argc % 2));
	assert(argc > 2);
	struct chunk chunk[argc / 2 - 1 ];
  //int fd = open("/dev/scd1", O_RDWR);
  //fprintf(stderr, "open cdvdrw as %d\n", fd);
  //fflush(stderr);
  SECURITY_ATTRIBUTES sa = {sizeof(sa), NULL, TRUE};
  HANDLE hVolWrite = CreateFile(argv[1],
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                              &sa, OPEN_EXISTING, 0, NULL);
  assert(hVolWrite != INVALID_HANDLE_VALUE );
  for(unsigned i = 0; i < sizeof(chunk) / sizeof(chunk[0]); ++i) {
	  char* endp;
	  chunk[i].offset = strtoul(argv[i*2 + 2], &endp, 0);
	  assert(endp && !*endp);
	  chunk[i].fp = fopen(argv[i*2 + 3], "rb");
	  assert(chunk[i].fp);
  }
  DWORD BytesReturned;
  BOOL locked = DeviceIoControl(hVolWrite, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &BytesReturned, NULL);
  if(!locked) {
    fprintf(stderr, "Failed to lock %s\n", argv[1]);
    fflush(stderr);
    exit(1);
  }
  BOOL unmount = DeviceIoControl(hVolWrite, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &BytesReturned, NULL);
  if(!unmount) {
    fprintf(stderr, "Failed to unmount %s\n", argv[1]);
    fflush(stderr);
    exit(2);
  }
  /*int fd = _open_osfhandle((intptr_t)hVolWrite, 0);
  assert(fd >=0 );
  FILE* fp = _fdopen(fd, "rb+");
  assert(fp);*/
  int fd = cygwin_attach_handle_to_fd("/dev/cdrom", -1, hVolWrite, 0, GENERIC_READ | GENERIC_WRITE);
  if(fd < 0) {
	fprintf(stderr, "Failed to open fd\n");
  } else {
	fprintf(stderr, "fd opened = %d\n", fd);
  }
  FILE* fp = fdopen(fd, "rb+");
  assert(fp);
  for(unsigned i = 0; i < sizeof(chunk) / sizeof(chunk[0]); ++i) {
	  int ret = fseek(fp, chunk[i].offset * sector_size, SEEK_CUR);
	  assert(!ret);
	  size_t s, t, a = 0;
	  do{
		  s = fread(buff, sector_size, sizeof(buff) /  sizeof(buff[0]), chunk[i].fp);
		  a += s;
		  t = fwrite(buff, sector_size, s, fp);
		  assert(s == t);
	  }while(s == sizeof(buff) /  sizeof(buff[0]));
	  assert(feof(chunk[i].fp));
	  assert(!ferror(chunk[i].fp));
	  fprintf(stderr, "finished chunk %u, offset %u, sectors %u\n", i, chunk[i].offset, (unsigned) a);
  }
  /*char buff[128];
  sprintf(buff, "%d", fd);
  setenv("CDVD_WRITE_FD_LOCKED", buff, 1);*/
  return 0;
}

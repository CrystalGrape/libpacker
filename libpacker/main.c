#include "libpack.h"

int main() {
	lpk_pack("E:/��Ŀ����/���п���/libpackerdemo/libpackerdemo/", strlen("E:/��Ŀ����/���п���/libpackerdemo/"), "E:/��Ŀ����/���п���/libpackerdemo/out.wpk");
	lpk_unpack("E:/��Ŀ����/���п���/libpackerdemo/out.wpk", "E:/��Ŀ����/���п���/unpack/");
	return 0;
}
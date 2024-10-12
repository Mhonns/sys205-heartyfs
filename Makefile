all:
	gcc -o bin/heartyfs_init src/heartyfs_init.c;
	gcc -o bin/heartyfs_mkdir src/op/heartyfs_mkdir.c;
	gcc -o bin/heartyfs_rmdir src/op/heartyfs_rmdir.c;
	gcc -o bin/heartyfs_creat src/op/heartyfs_creat.c;
	gcc -o bin/heartyfs_rm src/op/heartyfs_rm.c;
	gcc -o bin/heartyfs_read src/op/heartyfs_read.c;
	gcc -o bin/heartyfs_write src/op/heartyfs_write.c;

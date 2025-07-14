#include <ntifs.h>
#include <stdio.h>
extern "C" NTSTATUS DriverEntry() {
	printf("test");
	return STATUS_SUCCESS;
}
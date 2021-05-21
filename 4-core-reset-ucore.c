#include <assert.h>
#include <elf.h>
#include <fcntl.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define DDR_TOTAL_SIZE 0x80000000
#define DDR_BASE_ADDR ((uintptr_t)0x800000000)

#define GPIO_RESET_TOTAL_SIZE 0x1000
#define GPIO_RESET_BASE_ADDR 0x80010000

#define GPIO_NOHYPE_SETTINGS_SIZE 0x1000
#define GPIO_NOHYPE_SETTINGS_BASE 0x80004000

volatile void *ddr_base;
volatile uint32_t *gpio_reset_base;
volatile uint32_t *nohype_settings_base;
int fd;

void loader(char *imgfile, char *dtbfile, char *kernelfile, int offset) {
  FILE *fp = fopen(imgfile, "rb");
  assert(fp);
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  printf("image size = %ld\n", size);
  fseek(fp, 0, SEEK_SET);
  size_t nr_read = fread(ddr_base, size, 1, fp);
  printf("payload num %zd\n", nr_read);
  fclose(fp);

  fp = fopen(dtbfile, "rb");
  if (fp == NULL) {
    printf("No valid dtb file provided. Dtb in bootrom will be used.\n");
    return;
  }
  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  printf("dtb size = %ld\n", size);
  fseek(fp, 0, SEEK_SET);
  fread(ddr_base + 0x8, size, 1, fp);
  fclose(fp);

  fp = fopen(kernelfile, "rb");
  assert(fp);
  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  printf("kernel image size = %ld\n", size);
  fseek(fp, 0, SEEK_SET);
  nr_read = fread(ddr_base + offset, size, 1, fp);
  printf("payload num %zd\n", nr_read);
  fclose(fp);
}

void *create_map(size_t size, int fd, off_t offset) {
  volatile void *base = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);

  if (base == NULL) {
    perror("init_mem mmap failed:");
    close(fd);
    exit(1);
  }

  return base;
}

void init_map() {
  fd = open("/dev/mem", O_RDWR | O_SYNC);
  if (fd == -1) {
    perror("init_map open failed:");
    exit(1);
  }

  gpio_reset_base = create_map(GPIO_RESET_TOTAL_SIZE, fd, GPIO_RESET_BASE_ADDR);
  nohype_settings_base =
      create_map(GPIO_NOHYPE_SETTINGS_SIZE, fd, GPIO_NOHYPE_SETTINGS_BASE);
  ddr_base = create_map(DDR_TOTAL_SIZE, fd, DDR_BASE_ADDR);
}

void resetn(int val) { gpio_reset_base[0] = val; }
void nohypes(int val) { nohype_settings_base[0] = val; }

void finish_map() {
  munmap((void *)gpio_reset_base, GPIO_RESET_TOTAL_SIZE);
  munmap((void *)nohype_settings_base, GPIO_NOHYPE_SETTINGS_SIZE);
  munmap((void *)ddr_base, DDR_TOTAL_SIZE);
  close(fd);
}

int main(int argc, char *argv[]) {
  /* map some devices into the address space of this program */
  init_map();
  /*nohype gpio settings*/
  nohypes(0);
  /* reset RISC-V cores */
  resetn(0);

  printf("%s %s %s %s\n", argv[1], argv[2], argv[3], argv[4]);
  loader(argv[1], argv[2], argv[3], strtoll(argv[4], NULL, 16));

  /* finish resetting RISC-V cores */
  resetn(3);

  finish_map();

  return 0;
}

QEMU = qemu-system-riscv64

QEMUOPTS = -machine virt -kernel kernel.elf -nographic \
           -bios none -serial mon:stdio

all:
	riscv64-unknown-elf-gcc -Wall -Wextra -c -mcmodel=medany kernel.c -o kernel.o -ffreestanding
	riscv64-unknown-elf-as -c entry.s -o entry.o
	riscv64-unknown-elf-ld -T linker.ld -Lgcc -nostdlib kernel.o entry.o -o kernel.elf

qemu: all kernel.elf
	$(QEMU) $(QEMUOPTS)

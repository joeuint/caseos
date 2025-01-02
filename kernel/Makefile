CC = riscv64-unknown-elf-gcc
AS = riscv64-unknown-elf-as

QEMU = qemu-system-riscv64

QEMUOPTS = -machine virt -kernel kernel.elf -nographic \
           -bios none -serial mon:stdio

SRC = $(wildcard *.c)

ASM = $(wildcard *.s)

OBJ = $(SRC:.c=.o) $(ASM:.s=.o)  # Add entry.s to the OBJ list

LD_SCRIPT = linker.ld

TARGET = kernel.elf

LINKOPTS = -nostdlib -ffreestanding
COPTS = -ffreestanding -c -mcmodel=medany -Wall -Wextra

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -T $(LD_SCRIPT) -o $@ $^ $(LINKOPTS)

%.o: %.c
	$(CC) $(COPTS) $< -o $@

%.o: %.s
	$(AS) -c $< -o $@

qemu: all kernel.elf
	$(QEMU) $(QEMUOPTS)

clean:
	rm -vf $(OBJ)
	rm -vf $(TARGET)

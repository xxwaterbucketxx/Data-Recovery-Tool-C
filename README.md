# Data Recovery Tool (EXT3 File System)

A C-based digital forensics tool for recovering deleted PDF and GIF files from EXT3-formatted drives. This project demonstrates file carving and file system analysis techniques by locating file headers, parsing EXT3 inode structures, and reconstructing deleted files from block-level data.

## 📋 Overview

This tool recovers **deleted files** from an EXT3 filesystem by:
1. **Scanning** the drive for file signatures (magic bytes) to locate deleted file headers
2. **Parsing** EXT3 superblock and inode structures to determine block allocation
3. **Following** direct and indirect block pointers to reconstruct file data
4. **Writing** recovered data to output files (`output.pdf` or `output.gif`)

## ✨ Features

| Feature | Description |
|---------|-------------|
| **File Signature Detection** | Scans for GIF87a, GIF89a, and PDF (%PDF-) headers |
| **EXT3 Filesystem Parsing** | Reads superblock, block groups, and inode tables |
| **Block Recovery** | Handles both direct blocks (0-11) and indirect blocks |
| **EOF Detection** | Identifies end-of-file markers (%EOF for PDF, 0x3B00 for GIF) |
| **Block Size Calculation** | Dynamically determines block size from superblock |
| **MBR Validation** | Verifies Master Boot Record integrity (0x55AA signature) |

## 🏗️ Architecture

```
recoverFile.c (main)
    │
    ├── superBlock.c/h
    │   ├── getBlockSize()        → Reads block size from superblock
    │   ├── getPartAddr()         → Parses MBR partition table
    │   └── getSuperBlkAddr()     → Locates superblock for block group
    │
    ├── sigSearch.c/h
    │   └── getHead()             → Scans for file signature headers
    │
    └── recoveringFile.c/h
        └── writeOutput()         → Reconstructs file from blocks
            ├── Direct blocks (0-11)
            ├── Indirect block (index 12)
            └── writeLastBlock()  → Trims trailing zeros at EOF
```

## 📁 Project Structure

```
Data-Recovery-Tool-C/
├── recoverFile.c        # Main entry point
├── recoverFile.h        # (if exists)
├── superBlock.c         # EXT3 superblock parsing
├── superBlock.h         # Superblock structures/functions
├── sigSearch.c          # File signature scanning
├── sigSearch.h          # Signature definitions
├── recoveringFile.c     # File reconstruction logic
├── recoveringFile.h     # Recovery structures/functions
├── Makefile             # (optional - see build instructions)
├── output.pdf           # Sample recovered PDF
├── output.gif           # Sample recovered GIF
└── README.md            # This file
```

## 🚀 Getting Started

### Prerequisites
- Linux system (tested on Ubuntu/Debian)
- GCC compiler
- Root/sudo access (required for raw disk access)
- EXT3-formatted test drive with deleted PDF/GIF files

### Building

```bash
# Compile all source files
gcc -o file_recover *.c -lm

# Or with debugging symbols
gcc -g -o file_recover *.c -lm
```

### Usage

```bash
# Find your EXT3 drive
sudo fdisk -l

# Recover GIF file (type 1)
sudo ./file_recover /dev/sdX 1

# Recover PDF file (type 2)
sudo ./file_recover /dev/sdX 2
```

**Parameters:**
| Parameter | Description |
|-----------|-------------|
| `/dev/sdX` | Target block device (e.g., `/dev/sdb`, `/dev/sdc`) |
| `1` | Search for GIF files (outputs `output.gif`) |
| `2` | Search for PDF files (outputs `output.pdf`) |

### Example Session

```bash
$ sudo fdisk -l
# Identify your target drive, e.g., /dev/sdb

$ gcc -o file_recover *.c -lm

$ sudo ./file_recover /dev/sdb 2
Getting block size...
Getting PDF file header block number...
Copying deleted file inode index 0 block
Copying deleted file inode index 1 block
...
Searching for respective indirect block...
Respective indirect block found...
Writing last indirect block...
File was successfully recovered!

$ ls -la output.pdf
-rw-r--r-- 1 root root 1052352 Nov 5 12:09 output.pdf
```

## 🔧 Technical Details

### File Signatures Detected

| File Type | Header Signature | EOF Signature |
|-----------|------------------|---------------|
| **GIF87a** | `47 49 46 38 37 61` (GIF87a) | `3B 00` |
| **GIF89a** | `47 49 46 38 39 61` (GIF89a) | `3B 00` |
| **PDF** | `25 50 44 46 2D` (%PDF-) | `25 25 45 4F 46` (%EOF) |

### EXT3 Structures Parsed

1. **MBR (Master Boot Record)**
   - Validates `0x55AA` signature at offset 510
   - Reads partition table LBA addresses

2. **Superblock** (Block Group 0 at offset 1024)
   - Block size field (offset 0x18): `block_size = 2^(10 + size_field)`
   - Blocks per group (offset 0x20)
   - Magic signature `0xEF53` (offset 0x38)

3. **Inode Structure**
   - Direct blocks: indices 0-11
   - Indirect block: index 12 (contains block pointers)
   - Double/Triple indirect: not implemented (indices 13-14)

### Recovery Algorithm

```
1. Open device → Validate MBR → Locate partition start
2. Read superblock → Calculate block size
3. Scan blocks sequentially → Match file signatures
4. Found header → Calculate absolute offset
5. For each block in file:
   a. If direct block (0-11): Write entire block
   b. If indirect block (12):
      - Locate indirect block by relative numbering
      - Parse 4-byte block pointers
      - Convert relative → absolute block numbers (+256 offset)
      - Write each referenced block
   c. At EOF marker: Trim trailing zeros, write final partial block
```

## ⚠️ Limitations

| Limitation | Details |
|------------|---------|
| **File Types** | Only PDF and GIF (extensible via signatures) |
| **Indirect Blocks** | Single indirect only (no double/triple) |
| **Fragmentation** | Assumes contiguous allocation within blocks |
| **EXT3 Only** | Not compatible with EXT4, NTFS, FAT32 |
| **Deleted Files** | Requires inode not overwritten; works best on recently deleted files |
| **Root Required** | Needs `sudo` for raw block device access |

## 🧪 Testing

Create a test environment:
```bash
# Create test image
dd if=/dev/zero of=test.img bs=1M count=100
mkfs.ext3 test.img

# Mount and add files
sudo mount -o loop test.img /mnt/test
cp sample.pdf /mnt/test/
cp sample.gif /mnt/test/
sudo umount /mnt/test

# Delete files (simulate deletion)
# ... use debugfs or remount and rm ...

# Run recovery
sudo ./file_recover test.img 2  # for PDF
sudo ./file_recover test.img 1  # for GIF
```

## 📚 Learning Outcomes

This project demonstrates:
- **Digital Forensics** - File carving, slack space analysis
- **File Systems** - EXT3 on-disk structures (superblock, inodes, block groups)
- **Systems Programming** - Raw device I/O, `lseek`, `read`, `write`
- **Binary Parsing** - Little-endian hex to integer conversion
- **Memory Management** - Buffer handling, block-aligned reads
- **Error Handling** - Device validation, bounds checking

## 👨‍💻 Author

**Victor Vu** (NetID: vhv180000)  
CS 4398.001 - Digital Forensics Final Project

## 📄 License

Educational project - No license specified
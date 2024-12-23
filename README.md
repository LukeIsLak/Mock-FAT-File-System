# p3 : FAT System Interpreter
### Overview
This program is a FAT System Interpreter for CSC 360 Programming Assignment 3

### Features
The program is capable of the following feature:
    Diskinfo: displays information about the file system
    Disklist: displays the contents of the root directory or a given sub-directory in the file system.
    Diskget that copies a file from the file system image to the current directory in the host operating system.
    Diskput that copies a file from the current directory in the host operating system into the file system image, at the root directory or a given sub-directory.

### Technologies Used
- Project is done entirely in C, additional help with MAKEFILE

### Installation
1. **Prerequisites**: Install a C compiler such as gcc
2. **Clone the repository**:
    ```
    git clone https://gitlab.csc.uvic.ca/courses/2024091/CSC360_COSI/assignments/lukekuligowicz/p3.git
    ```

### Usage
1. **How to run the program**:

    using make, should get a ssi file
    ```
    make
    ```

    Upon, any of the 4 commands will be available
    ```
    ./diskinfo <.img filepath>
    ./disklist <.img filepath> <filepath within disklist to observe [stating at /]>
    ./diskget <.img filepath> <filepath within .img> <filepath to the output file>
    ./diskput <.img filepath> <filepath of the file to put in> <filepath within disk to put the file (should end with file extension)>
    ```

    Example (assuming test_input.txt and foo.txt exist within their proper places)
    ```
    ./diskinfo non-empty.img
    ./disklist non-empty.img /
    ./diskget non-empty.img /sub_Dir/foo.txt test_input.txt
    ./diskput non-empty.img test_input.txt /a/b/input.txt
    ```

    Furthermore, you can clean the compiled files using
    ```
    make clean
    ```

### File Structure
Main files (All created by Luke Kuligowicz)
    - main.c : main holder file, manages the program
    - diskinfo.h : main functions for ./diskinfo
    - disklist.h : main functions for ./disklist
    - diskget.h : main function for ./diskget
    - diskput.h : main functions for ./diskput
    - helperfunctions.h : reusable function used by more than one file
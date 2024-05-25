```
PatchPE 2.04 - Copyright (c) 2017-2024 by Javier Gutierrez Chamorro et al.
Patches PE headers to make them compatible with older versions of Windows
or, in the occasional case of data-only DLLs, make them work on Windows CE.

Usage: patchpe.exe <File> [<Options>]

Example: patchpe.exe notepad.exe /Default

Available options:
/Copy <File>                          Create and operate on a copy of the file
/Default                              Apply default modifications as of v1.35
/Preview                              Show but don't apply applicable changes
/Machine <Value>                      Modify IMAGE_FILE_HEADER accordingly
/Characteristics <Value>[:<Mask>]     Modify IMAGE_FILE_HEADER accordingly
/LinkerVersion <Value>                Modify IMAGE_OPTIONAL_HEADER accordingly
/Subsystem <Value>                    Modify IMAGE_OPTIONAL_HEADER accordingly
/DllCharacteristics <Value>[:<Mask>]  Modify IMAGE_OPTIONAL_HEADER accordingly
/OperatingSystemVersion <Value>       Modify IMAGE_OPTIONAL_HEADER accordingly
/ImageVersion <Value>                 Modify IMAGE_OPTIONAL_HEADER accordingly
/SubsystemVersion <Value>             Modify IMAGE_OPTIONAL_HEADER accordingly
/ImageBase <Value>                    Modify IMAGE_OPTIONAL_HEADER accordingly
/SizeOfStackReserve <Value>           Modify IMAGE_OPTIONAL_HEADER accordingly
/SizeOfStackCommit <Value>            Modify IMAGE_OPTIONAL_HEADER accordingly
/SizeOfHeapReserve <Value>            Modify IMAGE_OPTIONAL_HEADER accordingly
/SizeOfHeapCommit <Value>             Modify IMAGE_OPTIONAL_HEADER accordingly
/DependentLoadFlags <Value>[:<Mask>]> Modify IMAGE_LOAD_CONFIG_DIR accordingly
/Like <File>                          Use values from given PE file

If no options are given, no patching takes place.
If the /Copy option is given, it must precede all other options.
If the /Like option is given, subsequent options go without values.

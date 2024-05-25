// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// PatchPE 2.04 - Copyright (c) 2017-2024 by Javier Gutierrez Chamorro et al.
// Patches PE headers to make them compatible with older versions of Windows
// or, in the occasional case of data-only DLLs, make them work on Windows CE.
//
// SPDX-License-Identifier: MIT
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <windows.h>

struct PEHeader
{
	DWORD Signature;
	IMAGE_FILE_HEADER FileHeader;
	union
	{
		WORD Magic;
		struct { unsigned : 8, M32 : 1; };
		struct { unsigned : 9, M64 : 1; };
		struct { unsigned : 16, MajorLinkerVersion : 8; };
		struct { unsigned : 24, MinorLinkerVersion : 8; };
		IMAGE_OPTIONAL_HEADER32 OptionalHeader32;
		IMAGE_OPTIONAL_HEADER64 OptionalHeader64;
	};

	union
	{
		DWORD LoadConfigDirectory;
		IMAGE_LOAD_CONFIG_DIRECTORY32 LoadConfigDirectory32;
		IMAGE_LOAD_CONFIG_DIRECTORY64 LoadConfigDirectory64;
	};

	class IRef
	{
		// The capital I in the class name is meant to suggest that the class deals with numbers
		// of type ptrdiff_t or size_t, for which printf() reserves the format string modifier I.
		size_t const size;
		void *const p;
	public:
		template<typename T>
		IRef(T &r) : size(sizeof r), p(&r) { }
		operator ULONGLONG() { ULONGLONG v = 0; memcpy(&v, p, size); return v; }
		void operator=(ULONGLONG v) { memcpy(p, &v, size); }
	private:
		void operator=(IRef const &);
	};

	WORD &Subsystem()					{ return M32 ? OptionalHeader32.Subsystem					: OptionalHeader64.Subsystem;					}
	WORD &DllCharacteristics()			{ return M32 ? OptionalHeader32.DllCharacteristics			: OptionalHeader64.DllCharacteristics;			}
	DWORD &SectionAlignment()			{ return M32 ? OptionalHeader32.SectionAlignment			: OptionalHeader64.SectionAlignment;			}
	DWORD &FileAlignment()				{ return M32 ? OptionalHeader32.FileAlignment				: OptionalHeader64.FileAlignment;				}
	WORD &MajorOperatingSystemVersion() { return M32 ? OptionalHeader32.MajorOperatingSystemVersion	: OptionalHeader64.MajorOperatingSystemVersion;	}
	WORD &MinorOperatingSystemVersion() { return M32 ? OptionalHeader32.MinorOperatingSystemVersion	: OptionalHeader64.MinorOperatingSystemVersion;	}
	WORD &MajorImageVersion()			{ return M32 ? OptionalHeader32.MajorImageVersion			: OptionalHeader64.MajorImageVersion;			}
	WORD &MinorImageVersion()			{ return M32 ? OptionalHeader32.MinorImageVersion			: OptionalHeader64.MinorImageVersion;			}
	WORD &MajorSubsystemVersion()		{ return M32 ? OptionalHeader32.MajorSubsystemVersion		: OptionalHeader64.MajorSubsystemVersion;		}
	WORD &MinorSubsystemVersion()		{ return M32 ? OptionalHeader32.MinorSubsystemVersion		: OptionalHeader64.MinorSubsystemVersion;		}
	IRef ImageBase()					{ return M32 ? IRef(OptionalHeader32.ImageBase)				: IRef(OptionalHeader64.ImageBase);				}
	IRef SizeOfStackReserve()			{ return M32 ? IRef(OptionalHeader32.SizeOfStackReserve)	: IRef(OptionalHeader64.SizeOfStackReserve);	}
	IRef SizeOfStackCommit()			{ return M32 ? IRef(OptionalHeader32.SizeOfStackCommit)		: IRef(OptionalHeader64.SizeOfStackCommit);		}
	IRef SizeOfHeapReserve()			{ return M32 ? IRef(OptionalHeader32.SizeOfHeapReserve)		: IRef(OptionalHeader64.SizeOfHeapReserve);		}
	IRef SizeOfHeapCommit()				{ return M32 ? IRef(OptionalHeader32.SizeOfHeapCommit)		: IRef(OptionalHeader64.SizeOfHeapCommit);		}
	IMAGE_DATA_DIRECTORY *Directory()	{ return M32 ? OptionalHeader32.DataDirectory				: OptionalHeader64.DataDirectory;				}
	WORD &DependentLoadFlags()			{ return M32 ? LoadConfigDirectory32.DependentLoadFlags		: LoadConfigDirectory64.DependentLoadFlags;		}

	size_t FollowupSize() const
	{
		switch (Magic)
		{
		case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
			return sizeof OptionalHeader32 - sizeof Magic;
		case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
			return sizeof OptionalHeader64 - sizeof Magic;
		}
		return 0;
	}

	size_t LoadConfigSize() const
	{
		switch (Magic)
		{
		case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
			return sizeof LoadConfigDirectory32;
		case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
			return sizeof LoadConfigDirectory64;
		}
		return 0;
	}

};

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
extern "C" int wmain(int argc, WCHAR *argv[])
{
	IMAGE_DOS_HEADER udtDOSHeader;
	PEHeader udtPEHeader;
	size_t const preambleSize = RTL_SIZEOF_THROUGH_FIELD(PEHeader, Magic);

	wprintf(L"PatchPE 2.04 - Copyright (c) 2017-2024 by Javier Gutierrez Chamorro et al.\n"
			L"Patches PE headers to make them compatible with older versions of Windows\n"
			L"or, in the occasional case of data-only DLLs, make them work on Windows CE.\n\n");
	
	if (argc < 2)
	{
		wprintf(L"Usage: patchpe.exe <File> [<Options>]\n"
				L"\n"
				L"Example: patchpe.exe notepad.exe /Default\n"
				L"\n"
				L"Available options:\n"
				L"/Copy <File>                          Create and operate on a copy of the file\n"
				L"/Default                              Apply default modifications as of v1.35\n"
				L"/Preview                              Show but don't apply applicable changes\n"
				L"/Machine <Value>                      Modify IMAGE_FILE_HEADER accordingly\n"
				L"/Characteristics <Value>[:<Mask>]     Modify IMAGE_FILE_HEADER accordingly\n"
				L"/LinkerVersion <Value>                Modify IMAGE_OPTIONAL_HEADER accordingly\n"
				L"/Subsystem <Value>                    Modify IMAGE_OPTIONAL_HEADER accordingly\n"
				L"/DllCharacteristics <Value>[:<Mask>]  Modify IMAGE_OPTIONAL_HEADER accordingly\n"
				L"/OperatingSystemVersion <Value>       Modify IMAGE_OPTIONAL_HEADER accordingly\n"
				L"/ImageVersion <Value>                 Modify IMAGE_OPTIONAL_HEADER accordingly\n"
				L"/SubsystemVersion <Value>             Modify IMAGE_OPTIONAL_HEADER accordingly\n"
				L"/ImageBase <Value>                    Modify IMAGE_OPTIONAL_HEADER accordingly\n"
				L"/SizeOfStackReserve <Value>           Modify IMAGE_OPTIONAL_HEADER accordingly\n"
				L"/SizeOfStackCommit <Value>            Modify IMAGE_OPTIONAL_HEADER accordingly\n"
				L"/SizeOfHeapReserve <Value>            Modify IMAGE_OPTIONAL_HEADER accordingly\n"
				L"/SizeOfHeapCommit <Value>             Modify IMAGE_OPTIONAL_HEADER accordingly\n"
				L"/DependentLoadFlags <Value>[:<Mask>]> Modify IMAGE_LOAD_CONFIG_DIR accordingly\n"
				L"/Like <File>                          Use values from given PE file\n"
				L"\n"
				L"If no options are given, no patching takes place.\n"
				L"If the /Copy option is given, it must precede all other options.\n"
				L"If the /Like option is given, subsequent options go without values.\n");
		return(-1);
	}

	int i = 1;
	if (argc > 3 && _wcsicmp(argv[2], L"/Copy") == 0)
	{
		if (!CopyFileW(argv[1], argv[3], FALSE))
		{
			wprintf(L"Cannot copy %s to %s\n", argv[1], argv[3]);
			return(-2);
		}
		i = 3;
	}

	WCHAR *const r = argv[i];
	FILE *const pFile = _wfopen(r, argc > 2 ? L"r+b" : L"rb");
	if (!pFile)
	{
		wprintf(L"Cannot read input file %s\n", r);
		return(-3);
	}

	//Read DOS header
	if (fread(&udtDOSHeader, 1, sizeof(udtDOSHeader), pFile) != sizeof(udtDOSHeader))
	{
		wprintf(L"Cannot read DOS header %s\n", r);
		_fcloseall();
		return(-4);
	}
	if ((udtDOSHeader.e_magic != MAKEWORD('M','Z')) && (udtDOSHeader.e_magic != MAKEWORD('Z','M')))
	{
		wprintf(L"Not a valid DOS Executable %s\n", r);
		_fcloseall();
		return(-5);
	}

	//Read PE Header preamble
	if (fseek(pFile, udtDOSHeader.e_lfanew, SEEK_SET) != 0)
	{
		wprintf(L"Cannot seek to read PE header %s\n", r);
		_fcloseall();
		return(-6);
	}

	if (fread(&udtPEHeader, 1, preambleSize, pFile) != preambleSize)
	{
		wprintf(L"Cannot read PE header preamble %s\n", r);
		_fcloseall();
		return(-7);
	}
	if (udtPEHeader.Signature != MAKEWORD('P','E'))
	{
		wprintf(L"Not a valid PE Executable. Invalid PE header %s\n", r);
		_fcloseall();
		return(-8);
	}

	//Check PE Optional header and read followup bytes
	if (size_t const followupSize = udtPEHeader.FollowupSize())
	{
		if (fread(&udtPEHeader.Magic + 1, 1, followupSize, pFile) != followupSize)
		{
			wprintf(L"Cannot read PE header followup %s\n", r);
			_fcloseall();
			return(-9);
		}
	}
	else
	{
		wprintf(L"Not a valid PE Executable. Invalid optional PE header %s\n", r);
		_fcloseall();
		return(-10);
	}

	//Find and read load config
	WORD hasLoadConfig = udtPEHeader.FileHeader.NumberOfSections;
	IMAGE_DATA_DIRECTORY dSH = udtPEHeader.Directory()[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG];
	IMAGE_SECTION_HEADER eSH;
	do if (fread(&eSH, 1, sizeof eSH, pFile) != sizeof eSH)
	{
		wprintf(L"Cannot read section header %s\n", r);
		_fcloseall();
		return(-11);
	} while ((eSH.VirtualAddress > dSH.VirtualAddress || eSH.VirtualAddress + eSH.Misc.VirtualSize <= dSH.VirtualAddress) && --hasLoadConfig);

	if (hasLoadConfig)
	{
		if (fseek(pFile, dSH.VirtualAddress - eSH.VirtualAddress + eSH.PointerToRawData, SEEK_SET) != 0)
		{
			wprintf(L"Cannot seek to read load config %s\n", r);
			_fcloseall();
			return(-12);
		}
		if (size_t const loadConfigSize = udtPEHeader.LoadConfigSize())
		{
			if (fread(&udtPEHeader.LoadConfigDirectory, 1, loadConfigSize, pFile) != loadConfigSize)
			{
				wprintf(L"Cannot read load config %s\n", r);
				_fcloseall();
				return(-13);
			}
		}
		else
		{
			wprintf(L"Not a valid PE Executable. Invalid load config %s\n", r);
			_fcloseall();
			return(-14);
		}
	}

	//Patch header
	bool preview = false;
	FILE *pLike = NULL;
	PEHeader udtPEHeaderLike = udtPEHeader;
	PEHeader udtPEHeaderUnpatched = udtPEHeader;
	while (++i < argc)
	{
		WCHAR *s = argv[i];
		WCHAR *t = NULL;
		if (_wcsicmp(s, L"/Default") == 0)
		{
			//Apply default modifications as of v1.35
			udtPEHeader.MajorOperatingSystemVersion() = 4;
			udtPEHeader.MinorOperatingSystemVersion() = 0;
			udtPEHeader.MajorSubsystemVersion() = 4;
			udtPEHeader.MinorSubsystemVersion() = 0;
			udtPEHeader.FileHeader.Characteristics |= (IMAGE_FILE_LARGE_ADDRESS_AWARE | IMAGE_FILE_AGGRESIVE_WS_TRIM);
		}
		else if (_wcsicmp(s, L"/Preview") == 0)
		{
			preview = true;
		}
		else if (pLike || (++i < argc))
		{
			if (pLike == NULL)
				t = argv[i];
			if (t && _wcsicmp(s, L"/Like") == 0)
			{
				pLike = _wfopen(t, L"rb");
				if (!pLike)
				{
					wprintf(L"Cannot read input file %s\n", t);
					_fcloseall();
					return(-15);
				}

				//Read DOS header
				if (fread(&udtDOSHeader, 1, sizeof(udtDOSHeader), pLike) != sizeof(udtDOSHeader))
				{
					wprintf(L"Cannot read DOS header %s\n", t);
					_fcloseall();
					return(-16);
				}
				if ((udtDOSHeader.e_magic != MAKEWORD('M','Z')) && (udtDOSHeader.e_magic != MAKEWORD('Z','M')))
				{
					wprintf(L"Not a valid DOS Executable %s\n", t);
					_fcloseall();
					return(-17);
				}

				//Read PE Header
				if (fseek(pLike, udtDOSHeader.e_lfanew, SEEK_SET) != 0)
				{
					wprintf(L"Cannot seek to read PE header %s\n", t);
					_fcloseall();
					return(-18);
				}
				if (fread(&udtPEHeaderLike, 1, preambleSize, pLike) != preambleSize)
				{
					wprintf(L"Cannot read PE header preamble %s\n", t);
					_fcloseall();
					return(-19);
				}
				if (udtPEHeaderLike.Signature != MAKEWORD('P','E'))
				{
					wprintf(L"Not a valid PE Executable. Invalid PE header %s\n", t);
					_fcloseall();
					return(-20);
				}

				//Check PE Optional header and read followup bytes
				if (size_t const followupSize = udtPEHeaderLike.FollowupSize())
				{
					if (fread(&udtPEHeaderLike.Magic + 1, 1, followupSize, pLike) != followupSize)
					{
						wprintf(L"Cannot read PE header followup %s\n", r);
						_fcloseall();
						return(-21);
					}
				}
				else
				{
					wprintf(L"Not a valid PE Executable. Invalid optional PE header %s\n", r);
					_fcloseall();
					return(-22);
				}

				//Find and read load config
#				pragma warning(disable: 4456)
				WORD hasLoadConfig = udtPEHeaderLike.FileHeader.NumberOfSections;
				IMAGE_DATA_DIRECTORY dSH = udtPEHeaderLike.Directory()[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG];
				IMAGE_SECTION_HEADER eSH;
#				pragma warning(default: 4456)
				do if (fread(&eSH, 1, sizeof eSH, pLike) != sizeof eSH)
				{
					wprintf(L"Cannot read section header %s\n", r);
					_fcloseall();
					return(-23);
				} while ((eSH.VirtualAddress > dSH.VirtualAddress || eSH.VirtualAddress + eSH.Misc.VirtualSize <= dSH.VirtualAddress) && --hasLoadConfig);

				if (hasLoadConfig)
				{
					if (fseek(pLike, dSH.VirtualAddress - eSH.VirtualAddress + eSH.PointerToRawData, SEEK_SET) != 0)
					{
						wprintf(L"Cannot seek to read load config %s\n", r);
						_fcloseall();
						return(-24);
					}
					if (size_t const loadConfigSize = udtPEHeaderLike.LoadConfigSize())
					{
						if (fread(&udtPEHeaderLike.LoadConfigDirectory, 1, loadConfigSize, pLike) != loadConfigSize)
						{
							wprintf(L"Cannot read load config %s\n", r);
							_fcloseall();
							return(-25);
						}
					}
					else
					{
						wprintf(L"Not a valid PE Executable. Invalid load config %s\n", r);
						_fcloseall();
						return(-26);
					}
				}
			}
			else if (_wcsicmp(s, L"/Machine") == 0)
			{
				udtPEHeader.FileHeader.Machine = t ? static_cast<WORD>(wcstoul(t, &s, 0)) : udtPEHeaderLike.FileHeader.Machine;
			}
			else if (_wcsicmp(s, L"/Characteristics") == 0)
			{
				WORD value = t ? static_cast<WORD>(wcstoul(t, &s, 0)) : udtPEHeaderLike.FileHeader.Characteristics;
				if (s == t) value = 0xFFFF; //If no value is given, set the bits as per the given mask
				WORD const mask = *s == L':' ? static_cast<WORD>(wcstoul(s + 1, &s, 0)) : 0xFFFF;
				udtPEHeader.FileHeader.Characteristics = value & mask | ~mask & udtPEHeaderUnpatched.FileHeader.Characteristics;
			}
			else if (_wcsicmp(s, L"/LinkerVersion") == 0)
			{
				udtPEHeader.MajorLinkerVersion = t ? static_cast<BYTE>(wcstoul(t, &s, 10)) : udtPEHeaderLike.MajorLinkerVersion;
				s += wcsspn(s, L".");
				udtPEHeader.MinorLinkerVersion = t ? static_cast<BYTE>(wcstoul(s, &s, 10)) : udtPEHeaderLike.MinorLinkerVersion;
			}
			else if (_wcsicmp(s, L"/Subsystem") == 0)
			{
				udtPEHeader.Subsystem() = t ? static_cast<WORD>(wcstoul(t, &s, 0)) : udtPEHeaderLike.Subsystem();
			}
			else if (_wcsicmp(s, L"/DllCharacteristics") == 0)
			{
				WORD value = t ? static_cast<WORD>(wcstoul(t, &s, 0)) : udtPEHeaderLike.DllCharacteristics();
				if (s == t) value = 0xFFFF; //If no value is given, set the bits as per the given mask
				WORD const mask = *s == L':' ? static_cast<WORD>(wcstoul(s + 1, &s, 0)) : 0xFFFF;
				udtPEHeader.DllCharacteristics() = value & mask | ~mask & udtPEHeaderUnpatched.DllCharacteristics();
			}
			else if (_wcsicmp(s, L"/SectionAlignment") == 0)
			{
				udtPEHeader.SectionAlignment() = t ? wcstoul(t, &s, 0) : udtPEHeaderLike.SectionAlignment();
			}
			else if (_wcsicmp(s, L"/FileAlignment") == 0)
			{
				udtPEHeader.FileAlignment() = t ? wcstoul(t, &s, 0) : udtPEHeaderLike.FileAlignment();
			}
			else if (_wcsicmp(s, L"/OperatingSystemVersion") == 0)
			{
				udtPEHeader.MajorOperatingSystemVersion() = t ? static_cast<WORD>(wcstoul(t, &s, 10)) : udtPEHeaderLike.MajorOperatingSystemVersion();
				s += wcsspn(s, L".");
				udtPEHeader.MinorOperatingSystemVersion() = t ? static_cast<WORD>(wcstoul(s, &s, 10)) : udtPEHeaderLike.MinorOperatingSystemVersion();
			}
			else if (_wcsicmp(s, L"/ImageVersion") == 0)
			{
				udtPEHeader.MajorImageVersion() = t ? static_cast<WORD>(wcstoul(t, &s, 10)) : udtPEHeaderLike.MajorImageVersion();
				s += wcsspn(s, L".");
				udtPEHeader.MinorImageVersion() = t ? static_cast<WORD>(wcstoul(s, &s, 10)) : udtPEHeaderLike.MinorImageVersion();
			}
			else if (_wcsicmp(s, L"/SubsystemVersion") == 0)
			{
				udtPEHeader.MajorSubsystemVersion() = t ? static_cast<WORD>(wcstoul(t, &s, 10)) : udtPEHeaderLike.MajorSubsystemVersion();
				s += wcsspn(s, L".");
				udtPEHeader.MinorSubsystemVersion() = t ? static_cast<WORD>(wcstoul(s, &s, 10)) : udtPEHeaderLike.MinorSubsystemVersion();
			}
			else if (_wcsicmp(s, L"/ImageBase") == 0)
			{
				udtPEHeader.ImageBase() = t ? _wcstoui64(t, &s, 0) : udtPEHeaderLike.ImageBase();
			}
			else if (_wcsicmp(s, L"/SizeOfStackReserve") == 0)
			{
				udtPEHeader.SizeOfStackReserve() = t ? _wcstoui64(t, &s, 0) : udtPEHeaderLike.SizeOfStackReserve();
			}
			else if (_wcsicmp(s, L"/SizeOfStackCommit") == 0)
			{
				udtPEHeader.SizeOfStackCommit() = t ? _wcstoui64(t, &s, 0) : udtPEHeaderLike.SizeOfStackCommit();
			}
			else if (_wcsicmp(s, L"/SizeOfHeapReserve") == 0)
			{
				udtPEHeader.SizeOfHeapReserve() = t ? _wcstoui64(t, &s, 0) : udtPEHeaderLike.SizeOfHeapReserve();
			}
			else if (_wcsicmp(s, L"/SizeOfHeapCommit") == 0)
			{
				udtPEHeader.SizeOfHeapCommit() = t ? _wcstoui64(t, &s, 0) : udtPEHeaderLike.SizeOfHeapCommit();
			}
			else if (!hasLoadConfig)
			{
				s = t; // indicates invalid option
			}
			else if (_wcsicmp(s, L"/DependentLoadFlags") == 0)
			{
				WORD value = t ? static_cast<WORD>(wcstoul(t, &s, 0)) : udtPEHeaderLike.DependentLoadFlags();
				if (s == t) value = 0xFFFF; //If no value is given, set the bits as per the given mask
				WORD const mask = *s == L':' ? static_cast<WORD>(wcstoul(s + 1, &s, 0)) : 0xFFFF;
				udtPEHeader.DependentLoadFlags() = value & mask | ~mask & udtPEHeaderUnpatched.DependentLoadFlags();
			}
			else
			{
				s = t; // indicates invalid option
			}
		}
		else
		{
			s = t; // indicates invalid option
		}
		if (s == t)
		{
			wprintf(L"Invalid option\n");
			_fcloseall();
			return(-27);
		}
	}

	int const nAddressDigits = (udtPEHeader.M32 << 3) | (udtPEHeader.M64 << 4);

	wprintf(L"File:                               %s\n",		r);
	wprintf(L"Machine:                            0x%04X",		udtPEHeaderUnpatched.FileHeader.Machine);
	wprintf(udtPEHeader.FileHeader.Machine ==					udtPEHeaderUnpatched.FileHeader.Machine ? L"\n" : L" --> 0x%04X\n",
			udtPEHeader.FileHeader.Machine);
	wprintf(L"Bitness:                            %d\n",		nAddressDigits * 4);
	wprintf(L"Characteristics:                    0x%04X",		udtPEHeaderUnpatched.FileHeader.Characteristics);
	wprintf(udtPEHeader.FileHeader.Characteristics ==			udtPEHeaderUnpatched.FileHeader.Characteristics ? L"\n" : L" --> 0x%04X\n",
			udtPEHeader.FileHeader.Characteristics);
	wprintf(L"Linker Version:                     %u.%u",		udtPEHeaderUnpatched.MajorLinkerVersion,
																udtPEHeaderUnpatched.MinorLinkerVersion);
	wprintf(udtPEHeader.MajorLinkerVersion ==					udtPEHeaderUnpatched.MajorLinkerVersion &&
			udtPEHeader.MinorLinkerVersion ==					udtPEHeaderUnpatched.MinorLinkerVersion ? L"\n" : L" --> %u.%u\n",
			udtPEHeader.MajorLinkerVersion,
			udtPEHeader.MinorLinkerVersion);
	wprintf(L"Subsystem:                          %u",			udtPEHeaderUnpatched.Subsystem());
	wprintf(udtPEHeader.Subsystem() ==							udtPEHeaderUnpatched.Subsystem() ? L"\n" : L" --> %u\n",
			udtPEHeader.Subsystem());
	wprintf(L"Dll Characteristics:                0x%04X",		udtPEHeaderUnpatched.DllCharacteristics());
	wprintf(udtPEHeader.DllCharacteristics() ==					udtPEHeaderUnpatched.DllCharacteristics() ? L"\n" : L" --> 0x%04X\n",
			udtPEHeader.DllCharacteristics());
	wprintf(L"Section Alignment:                  0x%08X",		udtPEHeaderUnpatched.SectionAlignment());
	wprintf(udtPEHeader.SectionAlignment() ==					udtPEHeaderUnpatched.SectionAlignment() ? L"\n" : L" --> 0x%08X\n",
			udtPEHeader.SectionAlignment());
	wprintf(L"File Alignment:                     0x%08X",		udtPEHeaderUnpatched.FileAlignment());
	wprintf(udtPEHeader.FileAlignment() ==						udtPEHeaderUnpatched.FileAlignment() ? L"\n" : L" --> 0x%08X\n",
			udtPEHeader.FileAlignment());
	wprintf(L"Operating System Version:           %u.%u",		udtPEHeaderUnpatched.MajorOperatingSystemVersion(),
																udtPEHeaderUnpatched.MinorOperatingSystemVersion());
	wprintf(udtPEHeader.MajorOperatingSystemVersion() ==		udtPEHeaderUnpatched.MajorOperatingSystemVersion() &&
			udtPEHeader.MinorOperatingSystemVersion() ==		udtPEHeaderUnpatched.MinorOperatingSystemVersion() ? L"\n" : L" --> %u.%u\n",
			udtPEHeader.MajorOperatingSystemVersion(),
			udtPEHeader.MinorOperatingSystemVersion());
	wprintf(L"Image Version:                      %u.%u",		udtPEHeaderUnpatched.MajorImageVersion(),
																udtPEHeaderUnpatched.MinorImageVersion());
	wprintf(udtPEHeader.MajorImageVersion() ==					udtPEHeaderUnpatched.MajorImageVersion() &&
			udtPEHeader.MinorImageVersion() ==					udtPEHeaderUnpatched.MinorImageVersion() ? L"\n" : L" --> %u.%u\n",
			udtPEHeader.MajorImageVersion(),
			udtPEHeader.MinorImageVersion());
	wprintf(L"Subsystem Version:                  %u.%u",		udtPEHeaderUnpatched.MajorSubsystemVersion(),
																udtPEHeaderUnpatched.MinorSubsystemVersion());
	wprintf(udtPEHeader.MajorSubsystemVersion() ==				udtPEHeaderUnpatched.MajorSubsystemVersion() &&
			udtPEHeader.MinorSubsystemVersion() ==				udtPEHeaderUnpatched.MinorSubsystemVersion() ? L"\n" : L" --> %u.%u\n",
			udtPEHeader.MajorSubsystemVersion(),
			udtPEHeader.MinorSubsystemVersion());
	wprintf(L"Image Base:                         0x%0*I64X",	nAddressDigits, ULONGLONG(udtPEHeaderUnpatched.ImageBase()));
	wprintf(				ULONGLONG(udtPEHeader.ImageBase()) ==				ULONGLONG(udtPEHeaderUnpatched.ImageBase()) ? L"\n" : L" --> 0x%0*I64X\n",
			nAddressDigits, ULONGLONG(udtPEHeader.ImageBase()));
	wprintf(L"Size Of Stack Reserve:              0x%0*I64X",	nAddressDigits, ULONGLONG(udtPEHeaderUnpatched.SizeOfStackReserve()));
	wprintf(				ULONGLONG(udtPEHeader.SizeOfStackReserve()) ==		ULONGLONG(udtPEHeaderUnpatched.SizeOfStackReserve()) ? L"\n" : L" --> 0x%0*I64X\n",
			nAddressDigits, ULONGLONG(udtPEHeader.SizeOfStackReserve()));
	wprintf(L"Size Of Stack Commit:               0x%0*I64X",	nAddressDigits, ULONGLONG(udtPEHeaderUnpatched.SizeOfStackCommit()));
	wprintf(				ULONGLONG(udtPEHeader.SizeOfStackCommit()) ==		ULONGLONG(udtPEHeaderUnpatched.SizeOfStackCommit()) ? L"\n" : L" --> 0x%0*I64X\n",
			nAddressDigits, ULONGLONG(udtPEHeader.SizeOfStackCommit()));
	wprintf(L"Size Of Heap Reserve:               0x%0*I64X",	nAddressDigits, ULONGLONG(udtPEHeaderUnpatched.SizeOfHeapReserve()));
	wprintf(				ULONGLONG(udtPEHeader.SizeOfHeapReserve()) ==		ULONGLONG(udtPEHeaderUnpatched.SizeOfHeapReserve()) ? L"\n" : L" --> 0x%0*I64X\n",
			nAddressDigits, ULONGLONG(udtPEHeader.SizeOfHeapReserve()));
	wprintf(L"Size Of Heap Commit:                0x%0*I64X",	nAddressDigits, ULONGLONG(udtPEHeaderUnpatched.SizeOfHeapCommit()));
	wprintf(				ULONGLONG(udtPEHeader.SizeOfHeapCommit()) ==		ULONGLONG(udtPEHeaderUnpatched.SizeOfHeapCommit()) ? L"\n" : L" --> 0x%0*I64X\n",
			nAddressDigits, ULONGLONG(udtPEHeader.SizeOfHeapCommit()));

	WORD wChanged = udtPEHeader.FileHeader.Characteristics ^ udtPEHeaderUnpatched.FileHeader.Characteristics;
	wprintf(L"Large Address Aware:                %s",		udtPEHeaderUnpatched.FileHeader.Characteristics & IMAGE_FILE_LARGE_ADDRESS_AWARE ? L"Yes (>3GB)" : L"No (2GB)");
	wprintf((wChanged & IMAGE_FILE_LARGE_ADDRESS_AWARE) ==	0 ? L"\n" : L" --> %s\n",
			udtPEHeader.FileHeader.Characteristics & IMAGE_FILE_LARGE_ADDRESS_AWARE ? L"Yes (>3GB)" : L"No (2GB)");
	wprintf(L"Aggressively trim the working set:  %s",		udtPEHeaderUnpatched.FileHeader.Characteristics & IMAGE_FILE_AGGRESIVE_WS_TRIM ? L"Yes" : L"No");
	wprintf((wChanged & IMAGE_FILE_AGGRESIVE_WS_TRIM) ==	0 ? L"\n" : L" --> %s\n",
			udtPEHeader.FileHeader.Characteristics & IMAGE_FILE_LARGE_ADDRESS_AWARE ? L"Yes" : L"No");

	wprintf(!hasLoadConfig ?
			L"Dependent Load Flags:               N/A" :
			L"Dependent Load Flags:               0x%04X",		udtPEHeaderUnpatched.DependentLoadFlags());
	wprintf(udtPEHeader.DependentLoadFlags() ==					udtPEHeaderUnpatched.DependentLoadFlags() ? L"\n" : L" --> 0x%04X\n",
			udtPEHeader.DependentLoadFlags());

	//Write PE header if there were changes
	if (preview)
	{
		wprintf(L"\nNot patched due to preview mode!\n\n");
	}
	else if (memcmp(&udtPEHeaderUnpatched, &udtPEHeader, sizeof udtPEHeader))
	{
		if (fseek(pFile, udtDOSHeader.e_lfanew, SEEK_SET) != 0)
		{
			wprintf(L"Cannot seek to write new PE header %s\n", r);
			_fcloseall();
			return(-28);
		}
		if (fwrite(&udtPEHeader, 1, preambleSize, pFile) != preambleSize)
		{
			wprintf(L"Cannot write new header preamble %s\n", r);
			_fcloseall();
			return(-29);
		}
		if (size_t const followupSize = udtPEHeader.FollowupSize())
		{
			if (fwrite(&udtPEHeader.Magic + 1, 1, followupSize, pFile) != followupSize)
			{
				wprintf(L"Cannot write new header followup %s\n", r);
				_fcloseall();
				return(-30);
			}
		}
		if (hasLoadConfig)
		{
			if (fseek(pFile, dSH.VirtualAddress - eSH.VirtualAddress + eSH.PointerToRawData, SEEK_SET) != 0)
			{
				wprintf(L"Cannot seek to write new load config %s\n", r);
				_fcloseall();
				return(-31);
			}
			if (size_t const loadConfigSize = udtPEHeader.LoadConfigSize())
			{
				if (fwrite(&udtPEHeader.LoadConfigDirectory, 1, loadConfigSize, pFile) != loadConfigSize)
				{
					wprintf(L"Cannot write new load config %s\n", r);
					_fcloseall();
					return(-32);
				}
			}
		}
		wprintf(L"\nPatched successfully!\n\n");
	}
	else
	{
		wprintf(L"\nNothing to patch!\n\n");
	}

	_fcloseall();	
	return(0);
}

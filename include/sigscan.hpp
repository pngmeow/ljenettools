// library done by RVA
#pragma once
#ifndef __BATTLEHOOK_HELPER_HPP__
#define __BATTLEHOOK_HELPER_HPP__
#include <string>
#include <Windows.h>
#include <winnt.h>
#include <algorithm>


// Helper Macros
#define in_range( x, a, b ) ( x >= a && x <= b )
#define get_bits( x ) ( in_range( ( x & ( ~0x20 ) ), 'A', 'F' ) ? ( ( x & ( ~0x20 ) ) - 'A' + 0xA ) : ( in_range( x, '0', '9' ) ? x - '0' : 0 ) )
#define get_byte( x ) ( get_bits( x[ 0 ] ) << 4 | get_bits( x[ 1 ] ) )

namespace battlehook
{
	namespace structs
	{
		struct UNICODE_STRING
		{
			std::uint16_t length;
			std::uint16_t maximum_length;
			wchar_t* buffer;
		};

		struct LDR_DATA_TABLE_ENTRY
		{
			LIST_ENTRY in_load_order_links;
			LIST_ENTRY in_memory_order_links;
			LIST_ENTRY in_initialization_order_links;
			std::uintptr_t dll_base;
			std::uintptr_t entry_point;
			std::uint32_t size_of_image;
			char pad[0x10];
			UNICODE_STRING base_dll_name;
		};

		struct PEB_LDR_DATA
		{
			std::uint32_t length;
			bool initialized;
			char pad[0xB];
			LIST_ENTRY in_load_order_module_list;
		};

		struct PEB
		{
			char pad[0x10];
			std::uintptr_t image_base;
			PEB_LDR_DATA* peb_ldr;
		};
	}

	namespace helper
	{
		std::uintptr_t get_module_base(const char* module_name);
		std::uintptr_t find_pattern(const char* module_name, const char* pattern);
		std::uintptr_t resolve_rel(std::uintptr_t address, std::size_t len, std::size_t instruction);
	}
}


#endif // !__BATTLEHOOK_HELPER_HPP__
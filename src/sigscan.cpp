// library by RVA
#include "sigscan.hpp"

std::uintptr_t battlehook::helper::get_module_base(const char* module_name)
{
	const auto teb = NtCurrentTeb();
	if (!teb)
		return NULL;

	const auto peb = *reinterpret_cast<structs::PEB**>(reinterpret_cast<std::uintptr_t>(teb) + 0x60);
	if (!peb)
		return NULL;

	if(!module_name)
		return peb->image_base;

	const auto ldr = peb->peb_ldr;
	if (!ldr || !ldr->initialized)
		return NULL;


	const auto module_list = &ldr->in_load_order_module_list;
	for (auto entry = module_list->Flink; entry != module_list; entry = entry->Flink)
	{
		const auto data_table_entry = CONTAINING_RECORD(entry, structs::LDR_DATA_TABLE_ENTRY, in_load_order_links);
		std::wstring w_module_name(module_name, module_name + std::strlen(module_name));
		std::wstring w_dll_name(data_table_entry->base_dll_name.buffer, data_table_entry->base_dll_name.length / sizeof(wchar_t));

		std::transform(w_module_name.begin(), w_module_name.end(), w_module_name.begin(), ::towlower);
		std::transform(w_dll_name.begin(), w_dll_name.end(), w_dll_name.begin(), ::towlower);

		if (!w_dll_name.compare(w_module_name))
			return (data_table_entry->dll_base);
	}

	return 0;
}

std::uintptr_t battlehook::helper::find_pattern(const char* module_name, const char* pattern)
{
	const std::uintptr_t module_base = battlehook::helper::get_module_base(module_name);

	if (!module_base)
		return NULL;

	auto pat1 = const_cast<char*>(pattern);
	const std::uintptr_t range_start = (module_base);

	const auto pe_start = reinterpret_cast<IMAGE_DOS_HEADER*>(module_base);
	if (pe_start->e_magic != IMAGE_DOS_SIGNATURE)
		return NULL;

	const auto nt_header = reinterpret_cast<IMAGE_NT_HEADERS*>(module_base + pe_start->e_lfanew);
	if (nt_header->Signature != IMAGE_NT_SIGNATURE)
		return NULL;

	const std::uintptr_t image_end = range_start + nt_header->OptionalHeader.SizeOfImage;
	std::uintptr_t first_match = 0;

	for (std::uintptr_t current_address = range_start; current_address < image_end; current_address++)
	{
		if (!*pat1)
			return first_match;

		if (*reinterpret_cast<std::uint8_t*>(pat1) == '\?' || *reinterpret_cast<std::uint8_t*>(current_address) == get_byte(pat1))
		{
			if (!first_match)
				first_match = current_address;

			if (!pat1[2])
				return first_match;

			if (*reinterpret_cast<std::uint16_t*>(pat1) == '\?\?' || *reinterpret_cast<std::uint8_t*>(pat1) != '\?')
				pat1 += 3;
			else
				pat1 += 2;
		}
		else
		{
			pat1 = const_cast<char*>(pattern);
			first_match = 0;
		}
	}

	return NULL;
}

std::uintptr_t battlehook::helper::resolve_rel(std::uintptr_t address, std::size_t len, std::size_t instruction)
{
	const std::int32_t relative_offset = *reinterpret_cast<std::int32_t*>(address + instruction);
	return address + len + relative_offset;
}

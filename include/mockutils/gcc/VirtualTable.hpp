/*
 * Copyright (c) 2014 Eran Pe'er.
 *
 * This program is made available under the terms of the MIT License.
 *
 * Created on Mar 10, 2014
 */

#ifndef VirtualTable_h__
#define VirtualTable_h__

#include "mockutils/gcc/is_simple_inheritance_layout.hpp"
#include "mockutils/VTUtils.hpp"

namespace fakeit {

template<class C, class ... baseclasses>
struct VirtualTable {

	static_assert(is_simple_inheritance_layout<C>::value, "Can't mock a type with multiple inheritance");

	static VirtualTable<C, baseclasses...>* cloneVTable(C& instance) {
		fakeit::VirtualTable<C, baseclasses...>& orig = getVTable(instance);
		return orig.clone();
	}

	static VirtualTable<C, baseclasses...>& getVTable(C& instance) {
		fakeit::VirtualTable<C, baseclasses...>* vt = (fakeit::VirtualTable<C, baseclasses...>*)(&instance);
		return *vt;
	}

	VirtualTable<C, baseclasses...>* clone() {
		auto array = buildVTArray();
		int size = VTUtils::getVTSize<C>();
		auto firstMethod = array;
		firstMethod++; // skip top_offset
		firstMethod++; // skip type_info ptr
		firstMethod[-1] = this->firstMethod[-1]; // copy type_info
		for (int i = 0; i < size; i++) {
			firstMethod[i] = getMethod(i);
		}
		return new VirtualTable(array);
	}

	VirtualTable() :
			VirtualTable(buildVTArray()) {
	}

	~VirtualTable() {
		firstMethod--;
		firstMethod--;
		delete[] firstMethod;
	}

	void apply(C& instance) {
		int ** vtPtr = (int**)(&instance);
		*vtPtr = (int *)(this);
	}

	void setMethod(unsigned int index, void *method) {
		firstMethod[index] = method;
	}

	void * getMethod(unsigned int index) {
		return firstMethod[index];
	}

	unsigned int getSize() {
		return VTUtils::getVTSize<C>();
	}

	void initAll(void* value) {
		unsigned int size = getSize();
		for (unsigned int i = 0; i < size; i++) {
			setMethod(i, value);
		}
	}

	const std::type_info* getTypeId(){
		return (const std::type_info*)(firstMethod[-1]);
	}

private:
	void** firstMethod;

	static void ** buildVTArray() {
		int size = VTUtils::getVTSize<C>();
		auto array = new void*[size + 2] { };
		array[1] = (void*) &typeid(C);
		return array;
	}

	VirtualTable(void** vtarray) {
		firstMethod = vtarray;
		firstMethod++; // top_offset
		firstMethod++; // type_info ptr
	}

};
}
#endif // VirtualTable_h__

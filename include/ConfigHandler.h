/////////////////////////////////////////
//
//   OpenLieroX
//
//   Auxiliary Software class library
//
//   based on the work of JasonB
//   enhanced by Dark Charlie and Albert Zeyer
//
//   code under LGPL
//
/////////////////////////////////////////


// Config file handler
// Created 30/9/01
// By Jason Boettcher


#ifndef __CONFIGHANDLER_H__
#define __CONFIGHANDLER_H__

#include "CVec.h"
#include "StringUtils.h"
#include "MathLib.h"
#include <string>
#include <SDL.h>
#include <vector>

#define		MAX_STRING_LENGTH	4096
#define		MAX_MINOR_LENGTH	256
#define		MAX_KEYWORDS		256



// Value reading
bool	ReadString(const std::string& filename, const std::string& section, const std::string& key, std::string& value, std::string defaultv, bool abs_fn = false);
bool	ReadInteger(const std::string& filename, const std::string& section, const std::string& key, int *value, int defaultv);
bool	ReadFloat(const std::string& filename, const std::string& section, const std::string& key, float *value, float defaultv);
bool	ReadIntArray(const std::string& filename, const std::string& section, const std::string& key, int *array, int num_items);


struct Color;
bool	ReadColour(const std::string& filename, const std::string& section, const std::string& key, Color& value, const Color& defaultv);



template<typename T>
bool ReadArray(const std::string& filename, const std::string& section, const std::string& key, T* data, size_t num) {
	std::string string;
	
	if (!ReadString(filename,section,key,string,""))
		return false;
	
	std::vector<std::string> arr = explode(string,",");
	for (size_t i=0; i< MIN(num,arr.size()); i++)
		data[i] = from_string<T>(arr[i]);
	
	return num == arr.size();
}

template<typename T>
bool ReadVectorD2(const std::string& filename, const std::string& section, const std::string& key, VectorD2<T>& v, VectorD2<T> defv = VectorD2<T>()) {
	v = defv;
	
	T _v[2] = {0,0};
	if(!ReadArray(filename, section, key, _v, 2)) return false;
	
	v.x = _v[0]; v.y = _v[1];
	return true;
}

template<typename T>
bool ReadMatrixD2(const std::string& filename, const std::string& section, const std::string& key, MatrixD2<T>& v, MatrixD2<T> defv = MatrixD2<T>()) {
	v = defv;
	
	T _v[4] = {0,0,0,0};
	if(!ReadArray(filename, section, key, _v, 4)) return false;
	
	v.v1.x = _v[0]; v.v1.y = _v[1]; v.v2.x = _v[2]; v.v2.y = _v[3];
	return true;
}





bool	AddKeyword(const std::string& key, int value);
bool	ReadKeyword(const std::string& filename, const std::string& section, const std::string& key, int *value, int defaultv);
bool	ReadKeyword(const std::string& filename, const std::string& section, const std::string& key, bool *value, bool defaultv);
bool	ReadKeywordList(const std::string& filename, const std::string& section, const std::string& key, int *value, int defaultv);








#endif  //  __CONFIGHANDLER_H__

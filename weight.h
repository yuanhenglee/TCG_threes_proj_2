/**
 * Framework for Threes! and its variants (C++ 11)
 * weight.h: Lookup table template for n-tuple network
 *
 * Author: Theory of Computer Games
 *         Computer Games and Intelligence (CGI) Lab, NYCU, Taiwan
 *         https://cgilab.nctu.edu.tw/
 */

#pragma once
#include "board.h"
#include <iostream>
#include <vector>
#include <utility>
#include <assert.h>

class weight {
public:
	typedef float type;

public:
	weight() {}
	// weight(size_t len) : value(len) {}
	// weight(weight&& f) : value(std::move(f.value)) {}
	// weight(const weight& f) = default;
	//size of value: 16 ^ p.size()
	weight(const std::vector<int> &p) : pattern(p) {
		size_t value_size = 1 << (p.size() << 2);
		value = std::vector<type>(value_size, 0);
		for( int iso_idx = 0 ; iso_idx < 8 ; ++iso_idx ){
			std::vector<int> cur_iso;	
			for( auto &p : pattern ) {
				cur_iso.emplace_back( gen_isomorphism( p )[iso_idx]);
			}
			isomorphism.emplace_back( cur_iso );
		}

		// for( auto &iso : isomorphism ){
		// 	for( auto &p : iso )
		// 		std::cout<<p<<" ";
		// 	std::cout<<std::endl;
		// }
	}

	weight& operator =(const weight& f) = default;
	type& operator[] (size_t i) { return value[i]; }
	const type& operator[] (size_t i) const { return value[i]; }
	size_t size() const { return value.size(); }

public:

	float estimate( const board &b) const{
		float sum = 0;
		//iter through all isomorphisms
		for( auto &iso : isomorphism ){
			int value_idx = 0;
			for( size_t i = 0 ; i < iso.size() ; i++ ){
				int val = b(iso[i]);
				value_idx |= val << (i << 2);
			}
			sum += value[value_idx];
		}
		return sum;
	}

	float update( const board &b, float delta ){
		// if( delta > 0 ){
		// 	std::cout<<"weight update: "<<delta<<std::endl;
		// }
		float sum = 0;
		//iter through all isomorphisms
		for( auto &iso : isomorphism ){
			int value_idx = 0;
			for( size_t i = 0 ; i < iso.size() ; i++ ){
				int val = b(iso[i]);
				value_idx |= val << (i << 2);
			}
			value[value_idx] += delta;
			sum += value[value_idx];
		}
		return sum;
	}

	friend std::ostream& operator <<(std::ostream& out, const weight& w) {
		auto& value = w.value;
		uint64_t size = value.size();
		out.write(reinterpret_cast<const char*>(&size), sizeof(uint64_t));
		out.write(reinterpret_cast<const char*>(value.data()), sizeof(type) * size);
		return out;
	}
	friend std::istream& operator >>(std::istream& in, weight& w) {
		auto& value = w.value;
		uint64_t size = 0;
		in.read(reinterpret_cast<char*>(&size), sizeof(uint64_t));
		value.resize(size);
		in.read(reinterpret_cast<char*>(value.data()), sizeof(type) * size);
		return in;
	}

private:

	std::vector<int> gen_isomorphism( int idx ){
		switch(idx){
			case 0:
				return std::vector<int>{0, 3, 15, 12, 3, 15, 12, 0};
			case 1:
				return std::vector<int>{1, 7, 14, 8, 2, 11, 13, 4};
			case 2:
				return std::vector<int>{2, 11, 13, 4, 1, 7, 14, 8};
			case 3:
				return std::vector<int>{3, 15, 12, 0, 0, 3, 15, 12};
			case 4:
				return std::vector<int>{4, 2, 11, 13, 7, 14, 8, 1};
			case 5:
				return std::vector<int>{5, 6, 10, 9, 6, 10, 9, 5};
			case 6:
				return std::vector<int>{6, 10, 9, 5, 5, 6, 10, 9};
			case 7:
				return std::vector<int>{7, 14, 8, 1, 4, 2, 11, 13};
			case 8:
				return std::vector<int>{8, 1, 7, 14, 11, 13, 4, 2};
			case 9:
				return std::vector<int>{9, 5, 6, 10, 10, 9, 5, 6};
			case 10:
				return std::vector<int>{10, 9, 5, 6, 9, 5, 6, 10};
			case 11:
				return std::vector<int>{11, 13, 4, 2, 8, 1, 7, 14};
			case 12:
				return std::vector<int>{12, 0, 3, 15, 15, 12, 0, 3};
			case 13:
				return std::vector<int>{13, 4, 2, 11, 14, 8, 1, 7};
			case 14:
				return std::vector<int>{14, 8, 1, 7, 13, 4, 2, 11};
			case 15:
				return std::vector<int>{15, 12, 0, 3, 12, 0, 3, 15};
		}
		return std::vector<int>();
	}

protected:
	std::vector<type> value;
	std::vector<int> pattern;
	std::vector<std::vector<int>> isomorphism;
};

#pragma once

#include "Token.h"	
#include <string>
#include <map>
#include <memory>
#include <vector>

enum SymbolType {
	SYM_VAR,
};

struct SymbolEntry {
	std::string name;
	SymbolType type;
	TokenType declaredTokenType;

	SymbolEntry(std::string n, SymbolType st, TokenType tt) : name(std::move(n)), type(std::move(st)), declaredTokenType(std::move(tt))	{}
};

class SymbolTable {
public:
	SymbolTable() : outer(nullptr) {}
	SymbolTable(std::unique_ptr<SymbolTable> o) : outer(std::move(o)) {}

	bool define(const std::string& name, SymbolType symType, TokenType declaredType) {
		if (store.count(name)) {
			return false;
		}
		 
		store.emplace(name, SymbolEntry(name, symType, declaredType));
		return true;
	}

	SymbolEntry* resolve(const std::string& name) {
		if (store.count(name)) {
			return &store.at(name);
		}

		if (outer) {
			return outer->resolve(name);
		}

		return nullptr;
	}

	SymbolTable* getOuterPtr() const {
		return outer.get();
	}

	std::unique_ptr<SymbolTable> popOuterScope() {
		return std::move(outer);
	}

	const std::map<std::string, SymbolEntry>& getStore() const { return store; }

private:
	std::map<std::string, SymbolEntry> store;
	std::unique_ptr<SymbolTable> outer;
};
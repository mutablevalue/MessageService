#pragma once
#include <array>
#include <random>
#include <chrono>
#include <thread>
#include <iostream>
#include "profile.h"
#include "file.h"



struct Key {

	KArray KeyValue;

public:

	void PrintKey() {
		std::cout << std::hex << std::setfill('0');

		for (unsigned char byte : KeyValue)
			std::cout << std::setw(2) << static_cast<int>(byte);

		std::cout << std::dec << "\n";
	}

	KArray const GetKey() const {
		return KeyValue;
	}

	inline KArray GenerateKey() {
		std::array<char, 64> key{};
		std::random_device device;

		const auto CurrentTime = // 
			std::chrono::high_resolution_clock::now().time_since_epoch().count();

		const auto ThreadID = // thread idek
			static_cast<unsigned long long>(std::hash<std::thread::id>{}(std::this_thread::get_id()));

		const unsigned long long combined_seed =
			device() ^ CurrentTime ^ ThreadID;

		std::mt19937_64 engine(combined_seed); // 64 bit engine

		std::uniform_int_distribution<unsigned long long> distribution(0, UINT_FAST64_MAX);

		for (std::size_t offset = 0; offset < 64; offset += sizeof(unsigned long long)) {
			const unsigned long long value = distribution(engine);
			std::memcpy(key.data() + offset, &value, sizeof(value));
		}
		return key;
	}


	Key() {
		FileSystem File;
		if (!File.KeyExists()) {
			File.OpenKeyFile();
			KArray TempKey = GenerateKey();
			File.WriteKey(TempKey);
			KeyValue = File.LoadKey();
		}
		else {
			KeyValue = File.LoadKey();
		}
	}
};

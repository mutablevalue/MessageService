#pragma once
#include "input_utils.h"
#include "file.h"


// write profile.h and the core such that the input updates a users profile and the profile
// is prepared to move across networks

// update user commands will be called by the execute command


struct Profile {
	char Name[16];
	u32 Color;

public:
	explicit Profile(const char Value[16], const u32 CValue) : Color(CValue) {
		std::memcpy(Name, Value, sizeof(Name)); // mem copy for ease
	}

	Profile() {
		FileSystem File;
		if (!File.ProfileExists()) {
			const char* NewName = "default";
			const u32 Color = 0x0;
			const Profile TempProfile{ NewName, Color };
			File.OpenProfileFile();
			File.WriteProfile(TempProfile);
		}
		Profile TempProfile = File.LoadProfile();
		std::memcpy(Name, TempProfile.GetName(), sizeof(Name));
		Color = TempProfile.GetColor();
		File.Close();
	}

	void SetName(const char Value[16]) {
		std::memcpy(Name, Value, sizeof(Name)); // value is now useless
		FileSystem File;
		if (!File.ProfileExists()) {
			const u32 Color = 0x0;
			const Profile TempProfile{ Name, Color };
			File.OpenProfileFile();
			File.WriteProfile(TempProfile);
		}
		Profile TempProfile = File.LoadProfile(); // load the profile
		std::memcpy(TempProfile.Name, Name, sizeof(TempProfile.Name));
		File.WriteProfile(TempProfile); // write back the profile
		File.Close();
	}
	const char* GetName() const { return Name; }

	void SetColor(const u32 CValue) { 
	Color = CValue;  // cavlue is now useless
	FileSystem File;
	if (!File.ProfileExists()) {
		const char* NewName = "default";
		const Profile TempProfile{ NewName, Color };
		File.OpenProfileFile();
		File.WriteProfile(TempProfile);
	}
	Profile TempProfile = File.LoadProfile();
	TempProfile.Color = Color;
	File.WriteProfile(TempProfile); // write back the profile
	File.Close();
	}

	const u32 GetColor() const { return Color; }
};

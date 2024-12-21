#pragma once
#include "engine/iw8/lua_State.hpp"
#include "engine/iw8/luaL_Reg.hpp"
#include "memory/minhook.hpp"
#include <global/string_literal.hpp>

namespace Client::HookPlate {
	template <Common::StringLiteral Name, typename Type, typename ...Args>
	struct StdcallHook {
		using funcType = Type __stdcall(Args...);
		static funcType hkCallback;
		static inline funcType* m_Original;

		static inline const char* GetName() {
			return Name.m_Value;
		}
	};

	template <Common::StringLiteral Name, typename Type, typename ...Args>
	struct CdeclHook {
		using funcType = Type __cdecl(Args...);
		static funcType hkCallback;
		static inline funcType* m_Original;

		static inline const char* GetName() {
			return Name.m_Value;
		}
	};

	template <Common::StringLiteral Name, typename Type, typename ...Args>
	struct FastcallHook {
		using funcType = Type __fastcall(Args...);
		static funcType hkCallback;
		static inline funcType* m_Original;

		static inline const char* GetName() {
			return Name.m_Value;
		}
	};

	template <Common::StringLiteral Name, typename TypeDecl>
	struct FnHook {
		using funcType = TypeDecl;
		static funcType* hkCallback;
		static inline funcType* m_Original;

		static inline const char* GetName() {
			return Name.m_Value;
		}
	};

	template <Common::StringLiteral Name, Common::StringLiteral Target>
	struct LuaHook {
		using funcType = int __fastcall(IW8::lua_State*);
		static funcType hkCallback;
		static inline funcType* m_Original;

		static inline const char* GetName() {
			return Name.m_Value;
		}

		static inline const char* GetTarget() {
			return Target.m_Value;
		}
	};

	class LuaHookStore {
	public:
		using funcType = int __fastcall(IW8::lua_State*);

		struct HookData {
			const char* m_Name{};
			const char* m_Target{};
			funcType* m_Callback{};
			funcType** m_Original{};
			funcType** m_PtrToFunc{};

			HookData(const char* name, const char* target, funcType* callback, funcType** original) {
				this->m_Name = name;
				this->m_Target = target;
				this->m_Callback = callback;
				this->m_Original = original;
				this->m_PtrToFunc = nullptr;
			}

			void Hook(IW8::luaL_Reg* reg) {
				if (this->m_Original == nullptr || this->m_Callback == nullptr) {
					return;
				}

				*this->m_Original = reg->m_Func;
				reg->m_Func = this->m_Callback;
				this->m_PtrToFunc = &reg->m_Func;
				LOG("LuaHookStore", DEBUG, "Hooked {} ({}).", this->m_Name, this->m_Target);
			}

			void Unhook() {
				if (this->m_PtrToFunc == nullptr || this->m_Original == nullptr || *this->m_Original == nullptr) {
					return;
				}

				*this->m_PtrToFunc = *this->m_Original;
				*this->m_Original = nullptr;
				LOG("LuaHookStore", DEBUG, "Unhooked {} ({}).", this->m_Name, this->m_Target);
			}
		};

		template <typename T>
		void Register() {
			this->m_Hooks.push_back({
				T::GetName(),
				T::GetTarget(),
				&T::hkCallback,
				&T::m_Original
			});
		}

		HookData* FindHook(const char* target) {
			for (HookData& hk : this->m_Hooks) {
				if (strcmp(hk.m_Target, target) == 0) {
					return &hk;
				}
			}
			return nullptr;
		}
	private:
		std::vector<HookData> m_Hooks{};
	};
}
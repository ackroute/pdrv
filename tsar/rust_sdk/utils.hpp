#pragma once
#include <windows.h>
#include <memory>
#include <string>
#include <sstream>
#include <array>
#include <algorithm>
#include <optional>
#include <vector>
#include <atomic>
#include <codecvt>

#include "vector.hpp"
#include "classes.hpp"
#include "static.h"

std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

#define UTF8_TO_UTF16(str) converter.from_bytes(str).c_str( )
#define UTF16_TO_UTF8(str) converter.to_bytes(str).c_str( )

namespace utils
{
	namespace memory
	{		
		/*template <typename t> t read( std::uintptr_t const address )
		{
			return *reinterpret_cast< t* >( address );
		}

		template <typename t> void write( std::uintptr_t const address, t data )
		{
			*reinterpret_cast< t* >( address ) = data;
		}*/

		template <typename t, std::size_t n> t read_array( std::uintptr_t const address, std::array<std::uintptr_t, n>& offsets )
		{
			t final_address{};
			void* cached_address = nullptr;

			for ( const auto& element : offsets )
			{
				if ( cached_address && element == offsets.back( ) )
				{
					final_address = *reinterpret_cast< t* >( cached_address + element );
					break;
				}

				const auto ptr = *reinterpret_cast< void* >( address + element );

				if ( !ptr )
					break;

				cached_address = ptr;
			}

			return final_address;
		}

		template <std::size_t N>
		std::uint8_t* find_signature( const std::string_view module_name, const char( &signature )[ N ] )
		{		/* buck */
			std::array<std::optional<std::uint8_t>, N> bytes{};
			{
				std::vector<std::string> split_signature;

				std::string token;
				std::istringstream token_stream{ signature };

				while ( std::getline( token_stream, token, ' ' ) )
					split_signature.push_back( token );

				std::transform( split_signature.cbegin( ), split_signature.cend( ), bytes.begin( ),
								[ ]( const std::string& str_hex_val ) -> std::optional<std::uint8_t>
								{
									return str_hex_val.find( '?' ) == std::string::npos ? std::optional{ std::stoi( str_hex_val, 0, 16 ) } : std::nullopt;
								} );
			}

			std::uint8_t* result;
			{
				const auto module_start = reinterpret_cast< std::uint8_t* >( GetModuleHandleA( module_name.data( ) ) );

				std::uint8_t* module_end;

				{
					const auto dos_headers = reinterpret_cast< PIMAGE_DOS_HEADER >( module_start );

					if ( !dos_headers )
						return nullptr;

					const auto nt_headers = reinterpret_cast< PIMAGE_NT_HEADERS >( module_start + dos_headers->e_lfanew );

					if ( !nt_headers )
						return nullptr;

					module_end = module_start + nt_headers->OptionalHeader.SizeOfImage;
				}

				auto found = std::search( module_start, module_end,
										  bytes.cbegin( ), bytes.cend( ),
										  [ ]( std::uint8_t memory_byte, std::optional<std::uint8_t> signature_byte ) -> bool
										  {
											  return signature_byte.value_or( memory_byte ) == memory_byte;
										  } );

				result = ( found != module_end ? found : nullptr );
			}

			return result;
		}
	}
	namespace mono
	{
		char* get_class_name( void* const networkable_class )
		{
			return *reinterpret_cast< char** >( *reinterpret_cast< std::uintptr_t* >( networkable_class ) + 0x10 );
		}

		std::wstring to_wstring( mono_string* str )
		{
			return std::wstring( &str->buffer, str->size );
		}

		std::string to_string( mono_string* str )
		{
			return UTF16_TO_UTF8( to_wstring( str ) );
		}

		namespace transform
		{
			/*geo::vec3_t get_position( void* transform )
			{
				if ((uint64_t)transform < 0x1000)
					return {};

				geo::vec3_t position{};
		
				static const auto get_position_injected = reinterpret_cast< uint64_t( __fastcall* )( void*, geo::vec3_t& ) >( std::uintptr_t( GetModuleHandleA( "UnityPlayer.dll" ) ) + 0x9276a0 );
				
				if ((uint64_t)get_position_injected < 0x989680)
					return {};
				
				get_position_injected( transform, position );

				return position;
			}*/
		}
	}
	namespace math
	{
		constexpr auto r2d = 57.2957795131f; /* 180 / pi, used for conversion from radians to degrees */
		constexpr auto d2r = 0.01745329251f; /* pi / 180, used for conversion from degrees to radians */

		__forceinline geo::vec3_t calculate_angle( const geo::vec3_t& source, const geo::vec3_t& destination )
		{
			const auto direction = source - destination;

			return { std::asin( direction.y / direction.length( ) ) * r2d, -std::atan2( direction.x, -direction.z ) * r2d, 0.f };
		}
		__forceinline float calculate_fov( const geo::vec3_t& source, geo::vec3_t& aim_angle )
		{
			aim_angle -= source;

			if ( aim_angle.x > 180.f )
				aim_angle.x -= 360.f;
			else if ( aim_angle.x < -180.f )
				aim_angle.x += 360.f;
			
			if ( aim_angle.y > 180.f )
				aim_angle.y -= 360.f;
			else if ( aim_angle.y < -180.f )
				aim_angle.y += 360.f;

			aim_angle.make_absolute( );

			return aim_angle.x + aim_angle.y;
		}
	}
	namespace render
	{
		bool world_to_screen( base_camera* camerar, geo::vec3_t& world, geo::vec2_t* screen )
		{
			NULL_CHECK_RET(camerar) false;
			
			base_camera camera = read<base_camera>((uint64_t)camerar);
			const auto matrix = camera.view_matrix.transpose( );

			const geo::vec3_t translation = { matrix[ 3 ][ 0 ], matrix[ 3 ][ 1 ], matrix[ 3 ][ 2 ] };
			const geo::vec3_t up = { matrix[ 1 ][ 0 ], matrix[ 1 ][ 1 ], matrix[ 1 ][ 2 ] };
			const geo::vec3_t right = { matrix[ 0 ][ 0 ], matrix[ 0 ][ 1 ], matrix[ 0 ][ 2 ] };

			const auto w = translation.dot_product( world ) + matrix[ 3 ][ 3 ];

			if ( w < 0.1f )
				return false;

			const auto x = right.dot_product( world ) + matrix[ 0 ][ 3 ];
			const auto y = up.dot_product( world ) + matrix[ 1 ][ 3 ];

			*screen =
				{
					( 1920.f * 0.5f ) * ( 1.f + x / w ),
					( 1080.f * 0.5f ) * ( 1.f - y / w )
				};

			return true;
		}
	}
	namespace game 
	{
		model_transforms get_model_tranforms(base_player* entity) 
		{
			base_player pl = read<base_player>((uint64_t)entity);
			model ml = read<model>((uint64_t)pl.model);
			model_transforms mt = read<model_transforms>((uint64_t)ml.transforms);
			return mt;
		}
		
		unity_transform* get_head_transform(base_player* entity)
		{
			model_transforms mt = get_model_tranforms(entity);
			return mt.head;
		}

		unity_transform* get_neck_transform(base_player* entity)
		{
			model_transforms mt = get_model_tranforms(entity);
			return mt.neck;
		}

		bool is_local_player(player_model* model) 
		{
			return read<bool>((uint64_t)model + PLAYERMODEL_LOCAL);
		}
	}
}

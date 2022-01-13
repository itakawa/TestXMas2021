//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2008-2019 Ryo Suzuki
//	Copyright (c) 2016-2019 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# pragma once
# include "Siv3D/Fwd.hpp"
# include "Siv3D/PointVector.hpp"
# include "Siv3D/Array.hpp"
# include "Siv3D/Line3D.hpp"

# include "Siv3D/SIMD_Float4.hpp"
# include "Siv3D/Fwd.hpp"


namespace s3d
{
	class LineString3D : protected Array<Vec3>
	{
	private:

		using base_type = Array<Vec3>;

		[[nodiscard]] LineString3D _catmullRom(int32 interpolation, bool isClosed) const;

//		[[nodiscard]] Polygon _calculateBuffer(double distance, int32 quality, bool isClosed) const;

//		const LineString3D& _paint(Image& dst, int32 thickness, const Color& color, bool isClosed) const;

//		const LineString3D& _overwrite(Image& dst, int32 thickness, const Color& color, bool antialiased, bool isClosed) const;

		const LineString3D& _draw( const ColorF& color, bool isClosed) const;

		void _drawCatmullRom( const ColorF& color, int32 interpolation, bool isClosed) const;

	public:

		using typename base_type::value_type;
		using typename base_type::pointer;
		using typename base_type::const_pointer;
		using typename base_type::reference;
		using typename base_type::const_reference;
		using typename base_type::iterator;
		using typename base_type::const_iterator;
		using typename base_type::reverse_iterator;
		using typename base_type::const_reverse_iterator;
		using typename base_type::size_type;
		using typename base_type::difference_type;
		using typename base_type::allocator_type;

		using base_type::Array;
		using base_type::assign;
		using base_type::get_allocator;
		using base_type::at;
		using base_type::operator[];
		using base_type::front;
		using base_type::back;
		using base_type::data;
		using base_type::begin;
		using base_type::cbegin;
		using base_type::end;
		using base_type::cend;
		using base_type::rbegin;
		using base_type::crbegin;
		using base_type::rend;
		using base_type::crend;
		using base_type::empty;
		using base_type::size;
		using base_type::max_size;
		using base_type::reserve;
		using base_type::capacity;
		using base_type::shrink_to_fit;
		using base_type::clear;
		using base_type::insert;
		using base_type::emplace;
		using base_type::erase;
		using base_type::push_back;
		using base_type::emplace_back;
		using base_type::pop_back;
		using base_type::resize;

		using base_type::count;
		using base_type::count_if;
		using base_type::isEmpty;
		using base_type::operator bool;
		using base_type::release;
		using base_type::size_bytes;
		using base_type::push_front;
		using base_type::pop_front;
		using base_type::choice;
		using base_type::fill;
		using base_type::join;
		using base_type::remove;

		LineString3D() = default;

		LineString3D(const LineString3D& lines);

		LineString3D(LineString3D&& lines);
		
		explicit LineString3D(const Array<Vec3>& points);

		explicit LineString3D(Array<Vec3>&& points);

		LineString3D& operator =(const Array<Vec3>& other);

		LineString3D& operator =(Array<Vec3>&& other) noexcept;

		LineString3D& operator =(const LineString3D& other);

		LineString3D& operator =(LineString3D&& other) noexcept;

		void assign(const LineString3D& other);

		void assign(LineString3D&& other) noexcept;

		LineString3D& operator <<(const Vec3& value);

		void swap(LineString3D& other) noexcept;

		LineString3D& append(const Array<Vec3>& other);

		LineString3D& append(const LineString3D& other);

		LineString3D& remove(const Vec3& value);

		LineString3D& remove_at(size_t index);

		template <class Fty>
		LineString3D& remove_if(Fty f)
		{
			base_type::remove_if(f);

			return *this;
		}

		LineString3D& reverse();

		LineString3D& rotate(std::ptrdiff_t count = 1);

		LineString3D& shuffle();

		template <class URBG>
		LineString3D& shuffle(URBG&& rbg)
		{
			base_type::shuffle(std::forward<URBG>(rbg));

			return *this;
		}

		[[nodiscard]] LineString3D slice(size_t index) const;

		[[nodiscard]] LineString3D slice(size_t index, size_t length) const;

		[[nodiscard]] size_t num_lines() const noexcept;

		[[nodiscard]] Line3D line3d(size_t index) const;

		[[nodiscard]] LineString3D movedBy(double x, double y, double z) const;

		[[nodiscard]] LineString3D movedBy(const Vec3& v) const;

		LineString3D& moveBy(double x, double y, double z) noexcept;

		LineString3D& moveBy(const Vec3& v) noexcept;

		[[nodiscard]] RectF calculateBoundingRect() const noexcept;

//		template <class Shape2DType>
//		[[nodiscard]] bool intersects(const Shape2DType& shape) const
//		{
//			return Geometry2D::Intersect(*this, shape);
//		}

		[[nodiscard]] LineString3D densified(double maxDistance) const;

		[[nodiscard]] LineString3D catmullRom(int32 interpolation = 24) const;

		[[nodiscard]] LineString3D catmullRomClosed(int32 interpolation = 24) const;

		[[nodiscard]] Polygon calculateBuffer(double distance, int32 quality = 24) const;

		[[nodiscard]] Polygon calculateBufferClosed(double distance, int32 quality = 24) const;

		const LineString3D& paint(Image& dst, const Color& color) const;

		const LineString3D& paint(Image& dst, int32 thickness, const Color& color) const;

		const LineString3D& paintClosed(Image& dst, const Color& color) const;

		const LineString3D& paintClosed(Image& dst, int32 thickness, const Color& color) const;

		const LineString3D& overwrite(Image& dst, const Color& color, bool antialiased = true) const;

		const LineString3D& overwrite(Image& dst, int32 thickness, const Color& color, bool antialiased = true) const;

		const LineString3D& overwriteClosed(Image& dst, const Color& color, bool antialiased = true) const;

		const LineString3D& overwriteClosed(Image& dst, int32 thickness, const Color& color, bool antialiased = true) const;

		const LineString3D& draw( const ColorF& color = Palette::White) const;
/*
		const LineString3D& draw(const Mat4x4 &vp, double thickness, const ColorF& color = Palette::White) const;

		const LineString3D& draw(const Mat4x4 &vp, const LineStyle& style, double thickness, const ColorF& color = Palette::White) const;

		const LineString3D& drawClosed(const Mat4x4 &vp, const ColorF& color = Palette::White) const;

		const LineString3D& drawClosed(const Mat4x4 &vp, double thickness, const ColorF& color = Palette::White) const;

		const LineString3D& drawClosed(const Mat4x4 &vp, const LineStyle& style, double thickness, const ColorF& color = Palette::White) const;
*/

		void drawCatmullRom(const ColorF& color = Palette::White, int32 interpolation = 24) const;
/*
		void drawCatmullRom(const Mat4x4 &vp, double thickness, const ColorF& color = Palette::White, int32 interpolation = 24) const;

		void drawCatmullRom(const Mat4x4 &vp, const LineStyle& style, double thickness = 1.0, const ColorF& color = Palette::White, int32 interpolation = 24) const;

		void drawCatmullRomClosed(const Mat4x4 &vp, const ColorF& color = Palette::White, int32 interpolation = 24) const;

		void drawCatmullRomClosed(const Mat4x4 &vp, double thickness, const ColorF& color = Palette::White, int32 interpolation = 24) const;

		void drawCatmullRomClosed(const Mat4x4 &vp, const LineStyle& style, double thickness = 1.0, const ColorF& color = Palette::White, int32 interpolation = 24) const;
*/
		double fullLength = 0;
		Array<double> lineLength = {0};
		double updateDistance();
		Vec3 getCurvedPoint(double progress, size_t start = 0, size_t end = SIZE_MAX);
	};
}

//////////////////////////////////////////////////
//
//	Format
//
//////////////////////////////////////////////////

namespace s3d
{
	void Formatter(FormatData& formatData, const LineString3D& value);

	template <class CharType>
	inline std::basic_ostream<CharType>& operator <<(std::basic_ostream<CharType>& output, const LineString3D& value)
	{
		output << CharType('(');

		bool b = false;

		for (const auto& point : value)
		{
			if (std::exchange(b, true))
			{
				output << CharType(',');
			}

			output << point;
		}

		return output << CharType(')');
	}
}

//////////////////////////////////////////////////
//
//	Hash
//
//////////////////////////////////////////////////

namespace std
{
	template <>
	struct hash<s3d::LineString3D>
	{
		[[nodiscard]] size_t operator ()(const s3d::LineString3D& value) const noexcept
		{
			return s3d::Hash::FNV1a(value.data(), value.size_bytes());
		}
	};
}

//////////////////////////////////////////////////
//
//	Swap
//
//////////////////////////////////////////////////

namespace std
{
	inline void swap(s3d::LineString3D& a, s3d::LineString3D& b) noexcept
	{
		a.swap(b);
	}
}


#include "LineString3D.ipp"
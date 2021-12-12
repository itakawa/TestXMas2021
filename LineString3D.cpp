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

# include "LineString3D.hpp"
# include <Siv3D/Optional.hpp>
//# include <Siv3D/Rectangle.hpp>
# include <Siv3D/Spline.hpp>
# include <Siv3D/Polygon.hpp>
//# include <../Siv3D/src/Siv3D/Common/Siv3DEngine.hpp>
# include <Siv3D/Mat4x4.hpp>
# include <Siv3D/Graphics2D.hpp>
#include <Siv3D/EngineLog.hpp>

namespace s3d
{
	namespace detail
	{
		static constexpr bool Open = false;

		static constexpr bool Closed = true;
	}

	LineString3D::LineString3D(const LineString3D& lines)
		: base_type(lines.begin(), lines.end())
	{

	}

	LineString3D::LineString3D(LineString3D&& lines)
		: base_type(std::move(lines))
	{

	}

	LineString3D::LineString3D(const Array<Vec3>& points)
		: base_type(points.begin(), points.end())
	{

	}

	LineString3D::LineString3D(Array<Vec3>&& points)
		: base_type(std::move(points))
	{

	}

	LineString3D& LineString3D::operator =(const Array<Vec3>& other)
	{
		base_type::operator=(other);

		return *this;
	}

	LineString3D& LineString3D::operator =(Array<Vec3>&& other) noexcept
	{
		base_type::operator=(std::move(other));

		return *this;
	}

	LineString3D& LineString3D::operator =(const LineString3D& other)
	{
		base_type::operator=(other);

		return *this;
	}

	LineString3D& LineString3D::operator =(LineString3D&& other) noexcept
	{
		base_type::operator=(std::move(other));

		return *this;
	}

	void LineString3D::assign(const LineString3D& other)
	{
		base_type::operator=(other);
	}

	void LineString3D::assign(LineString3D&& other) noexcept
	{
		base_type::operator=(std::move(other));
	}

	LineString3D& LineString3D::operator <<(const Vec3& value)
	{
		base_type::push_back(value);

		return *this;
	}

	void LineString3D::swap(LineString3D& other) noexcept
	{
		base_type::swap(other);
	}

	LineString3D& LineString3D::append(const Array<Vec3>& other)
	{
		base_type::insert(end(), other.begin(), other.end());

		return *this;
	}

	LineString3D& LineString3D::append(const LineString3D& other)
	{
		base_type::insert(end(), other.begin(), other.end());

		return *this;
	}

	LineString3D& LineString3D::remove(const Vec3& value)
	{
		base_type::remove(value);

		return *this;
	}

	LineString3D& LineString3D::remove_at(const size_t index)
	{
		base_type::remove_at(index);

		return *this;
	}

	LineString3D& LineString3D::reverse()
	{
		base_type::reverse();

		return *this;
	}

	LineString3D& LineString3D::rotate(const std::ptrdiff_t count)
	{
		base_type::rotate(count);

		return *this;
	}

	LineString3D& LineString3D::shuffle()
	{
		base_type::shuffle();

		return *this;
	}

	LineString3D LineString3D::slice(const size_t index) const
	{
		return LineString3D(base_type::slice(index));
	}

	LineString3D LineString3D::slice(const size_t index, const size_t length) const
	{
		return LineString3D(base_type::slice(index, length));
	}

	size_t LineString3D::num_lines() const noexcept
	{
		return size() < 2 ? 0 : size() - 1;
	}

	Line3D LineString3D::line3d(const size_t index) const
	{
		return{ base_type::operator[](index), base_type::operator[](index + 1) };
	}

	LineString3D LineString3D::movedBy(const double x, const double y, double z) const
	{
		return LineString3D(*this).moveBy(x, y, z);
	}

	LineString3D LineString3D::movedBy(const Vec3& v) const
	{
		return movedBy(v.x, v.y, v.z);
	}

	LineString3D& LineString3D::moveBy(const double x, const double y, const double z) noexcept
	{
		for (auto& point : *this)
		{
			point.moveBy(x, y, z);
		}

		return *this;
	}

	LineString3D& LineString3D::moveBy(const Vec3& v) noexcept
	{
		return moveBy(v.x, v.y, v.z);
	}

	RectF LineString3D::calculateBoundingRect() const noexcept
	{
		if (isEmpty())
		{
			return RectF(0);
		}

		const Vec3* it = data();
		const Vec3* itEnd = it + size();

		double left = it->x;
		double top = it->y;
		double right = left;
		double bottom = top;
		++it;
		
		while (it != itEnd)
		{
			if (it->x < left)
			{
				left = it->x;
			}
			else if (right < it->x)
			{
				right = it->x;
			}

			if (it->y < top)
			{
				top = it->y;
			}
			else if (bottom < it->y)
			{
				bottom = it->y;
			}

			++it;
		}

		return RectF(left, top, right - left, bottom - top);
	}

	LineString3D LineString3D::catmullRom(const int32 interpolation) const
	{
		return _catmullRom(interpolation, detail::Open);
	}

	LineString3D LineString3D::catmullRomClosed(const int32 interpolation) const
	{
		return _catmullRom(interpolation, detail::Closed);
	}

	const LineString3D& LineString3D::draw(const ColorF& color) const
	{
		return _draw( color, detail::Open);
	}
	void LineString3D::drawCatmullRom( const ColorF& color, const int32 interpolation) const
	{
		_drawCatmullRom( color, interpolation, detail::Open);
	}


	enum MODEAXIS { X = 0,Y = 1,Z = 2 };

	float CubicPolynomial(const bool& use_cache,const MODEAXIS& axis,
							const float &x0, const float &x1,const float &x2,const float &x3,
							const float &dt0, const float &dt1,const float &dt2,const float &t)
	{
		static float c0[3], c1[3], c2[3], c3[3];
		if (!use_cache)//前回キャッシュと同じ場合は計算をキャンセル
		{
			float t0 = ((x1 - x0) / dt0 - (x2 - x0) / (dt0 + dt1) + (x2 - x1) / dt1) * dt1;
			float t1 = ((x2 - x1) / dt1 - (x3 - x1) / (dt1 + dt2) + (x3 - x2) / dt2) * dt1;
			c0[axis] = x1;
			c1[axis] = t0;
			c2[axis] = -3 * x1 + 3 * x2 - 2 * t0 - t1;
			c3[axis] = 2 * x1 - 2 * x2 + t0 + t1;
		}
		return c0[axis] + c1[axis]*t + c2[axis]*(t*t) + c3[axis]*(t*t*t);
	}

	Float3 CatmullRom3D(const Float3& p0, 
						const Float3& p1, 
						const Float3& p2, 
						const Float3& p3, 
						const float& weight)
	{
		static Float3 cache[4];
		static float dt0, dt1, dt2;

		bool use_cache = (cache[0] == p0 && cache[1] == p1 && cache[2] == p2 && cache[3] == p3);
		if (!use_cache)//前回キャッシュと同じ場合は計算をキャンセル
		{
			dt0 = powf( p0.distanceFromSq( p1 ), 0.25f);
			dt1 = powf( p1.distanceFromSq( p2 ), 0.25f);
			dt2 = powf( p2.distanceFromSq( p3 ), 0.25f);
			cache[0] = p0; 
			cache[1] = p1; 
			cache[2] = p2; 
			cache[3] = p3;
		}

		float x = CubicPolynomial(use_cache, MODEAXIS::X, p0.x, p1.x, p2.x, p3.x, dt0, dt1, dt2, weight);
		float y = CubicPolynomial(use_cache, MODEAXIS::Y, p0.y, p1.y, p2.y, p3.y, dt0, dt1, dt2, weight);
		float z = CubicPolynomial(use_cache, MODEAXIS::Z, p0.z, p1.z, p2.z, p3.z, dt0, dt1, dt2, weight);
		return Float3 { x,y,z };
	}


	LineString3D LineString3D::_catmullRom(const int32 interpolation, const bool isClosed) const
	{
		if (size() < 2)
		{
			return *this;
		}

		// [Siv3D ToDo] 最適化

		Array<Vec3> ps;
		{
			ps.reserve(size() + 2 + isClosed);

			if (isClosed)
			{
				ps.push_back((*this)[size() - 1]);
			}
			else
			{
//				ps.push_back((*this)[0]);	//始端重複止め
				ps.push_back((*this)[0]-(*this)[1]+(*this)[0]);	//始端延長
			}

			for (const auto& point : *this)
			{
				ps.push_back(point);
			}

			if (isClosed)
			{
				ps.push_back((*this)[0]);
				ps.push_back((*this)[1]);
			}
			else
			{
//				ps.push_back((*this)[size() - 1]);	//終端重複止め
				ps.push_back((*this)[size() - 2]-(*this)[size() - 1]+(*this)[size() - 1]);	//終端延長
			}
		}

		LineString3D splinePoints;
		{
			splinePoints.reserve((ps.size() - 3)*interpolation + 1);

			for (size_t i = 1; i < ps.size() - 2; ++i)
			{
				const bool isLast = (i + 1) == ps.size() - 2;

				for (int32 t = 0; t < (interpolation + isLast); ++t)
				{
//2D
//					const Vec3 p = Spline::CatmullRom(ps[i - 1], ps[i], ps[i + 1], ps[i + 2], t / static_cast<double>(interpolation));

//3D
					const Vec3& p0 = ps[i-1];
					const Vec3& p1 = ps[i+0];
					const Vec3& p2 = ps[i+1];
					const Vec3& p3 = ps[i+2];
			        Vec3 p = CatmullRom3D(p0,p1,p2,p3, t / static_cast<double>(interpolation));

					splinePoints.push_back(p);
				}
			}
		}

		return splinePoints;
	}

	double LineString3D::updateDistance()
	{
		if ( this->num_lines() < 1) assert(1);

		fullLength = 0;
		for (uint32 i = 0; i < this->num_lines(); i++)
		{
			double len = this->at(i).length();
			lineLength.emplace_back( len );
			fullLength += len;
		}
		return fullLength;
	}

	Vec3 LineString3D::getCurvedPoint( double progress, size_t start, size_t end )
	{
		if ( start > end ) assert(1);
		if ( end-start < 1 ) assert(1);
		if ( end == SIZE_MAX) end = this->num_lines() - 1;

		if ( fullLength == 0 )
			if ( 0 == updateDistance() ) assert(1);
		double t = (end - start) * progress;
		size_t ti = (size_t)t;
		double tw = t - ti;

//		LOG_INFO( U"ti={} tw={:.02f}"_fmt(ti,tw) );

		Vec3 p0;
		if( ti==0 ) p0 = this->at(ti + 0);
		else        p0 = this->at(ti - 1);
		const Vec3& p1 = this->at(ti + 0);
		const Vec3& p2 = this->at(ti + 1);
		const Vec3& p3 = this->at(ti + 2);
		Vec3 p = CatmullRom3D(p0, p1, p2, p3, tw);

		return p;
	}

	const LineString3D& LineString3D::_draw( const ColorF& color, const bool isClosed) const
	{
		if (size() < 2)
		{
			return *this;
		}

		for (uint32 i = 1; i < size(); i += 2)
		{
			Line3D(data()[i + 0], data()[i + 1]).draw(color) ;
		}

		return *this;
	}

	void LineString3D::_drawCatmullRom( const ColorF& color, const int32 interpolation, const bool isClosed) const
	{
		_catmullRom(interpolation, isClosed)._draw( color, isClosed );
	}


	void Formatter( FormatData& formatData, const LineString3D& value )
	{
		formatData.string.append(value.join(U", "_sv, U"("_sv, U")"_sv));
	}
}

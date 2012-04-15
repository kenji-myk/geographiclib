/**
 * \file OSGB.hpp
 * \brief Header for GeographicLib::OSGB class
 *
 * Copyright (c) Charles Karney (2010, 2011) <charles@karney.com> and licensed
 * under the MIT/X11 License.  For more information, see
 * http://geographiclib.sourceforge.net/
 **********************************************************************/

#if !defined(GEOGRAPHICLIB_OSGB_HPP)
#define GEOGRAPHICLIB_OSGB_HPP "$Id$"

#include <string>
#include <sstream>
#include <GeographicLib/Constants.hpp>
#include <GeographicLib/TransverseMercator.hpp>

#if defined(_MSC_VER)
// Squelch warnings about dll vs string
#pragma warning (push)
#pragma warning (disable: 4251)
#endif

namespace GeographicLib {

  /**
   * \brief Ordnance Survey grid system for Great Britain
   *
   * The class implements the coordinate system used by the Ordnance Survey for
   * maps of Great Britain and conversions to the grid reference system.
   *
   * See
   * - <a href="http://www.ordnancesurvey.co.uk/oswebsite/gps/docs/A_Guide_to_Coordinate_Systems_in_Great_Britain.pdf">
   * A guide to coordinate systems in Great Britain</a>
   * - <a href="http://www.ordnancesurvey.co.uk/oswebsite/gps/information/coordinatesystemsinfo/guidetonationalgrid/page1.html">
   * Guide to National Grid</a>
   *
   * \b WARNING: the latitudes and longitudes for the Ordnance Survey grid
   * system do not use the WGS84 datum.  Do not use the values returned by this
   * class in the UTMUPS, MGRS, or Geoid classes without first converting the
   * datum (and vice versa).
   *
   * Example of use:
   * \include example-OSGB.cpp
   **********************************************************************/
  class GEOGRAPHIC_EXPORT OSGB {
  private:
    typedef Math::real real;
    static const std::string letters_;
    static const std::string digits_;
    static const TransverseMercator OSGBTM_;
    static const real northoffset_;
    enum {
      base_ = 10,
      tile_ = 100000,
      tilelevel_ = 5,
      tilegrid_ = 5,
      tileoffx_ = 2 * tilegrid_,
      tileoffy_ = 1 * tilegrid_,
      minx_ = - tileoffx_ * tile_,
      miny_ = - tileoffy_ * tile_,
      maxx_ = (tilegrid_*tilegrid_ - tileoffx_) * tile_,
      maxy_ = (tilegrid_*tilegrid_ - tileoffy_) * tile_,
      // Maximum precision is um
      maxprec_ = 5 + 6,
    };
    static real computenorthoffset() throw();
    static void CheckCoords(real x, real y);
    OSGB();                     // Disable constructor

  public:

    /**
     * Forward projection, from geographic to OSGB coordinates.
     *
     * @param[in] lat latitude of point (degrees).
     * @param[in] lon longitude of point (degrees).
     * @param[out] x easting of point (meters).
     * @param[out] y northing of point (meters).
     * @param[out] gamma meridian convergence at point (degrees).
     * @param[out] k scale of projection at point.
     *
     * \e lat should be in the range [-90, 90]; \e lon and \e lon0 should be in
     * the range [-180, 360].
     **********************************************************************/
    static void Forward(real lat, real lon,
                        real& x, real& y, real& gamma, real& k) throw() {
      OSGBTM_.Forward(OriginLongitude(), lat, lon, x, y, gamma, k);
      x += FalseEasting();
      y += northoffset_;
    }

    /**
     * Reverse projection, from OSGB coordinates to geographic.
     *
     * @param[in] x easting of point (meters).
     * @param[in] y northing of point (meters).
     * @param[out] lat latitude of point (degrees).
     * @param[out] lon longitude of point (degrees).
     * @param[out] gamma meridian convergence at point (degrees).
     * @param[out] k scale of projection at point.
     *
     * The value of \e lon returned is in the range [-180, 180).
     **********************************************************************/

    static void Reverse(real x, real y,
                        real& lat, real& lon, real& gamma, real& k) throw() {
      x -= FalseEasting();
      y -= northoffset_;
      OSGBTM_.Reverse(OriginLongitude(), x, y, lat, lon, gamma, k);
    }

    /**
     * OSGB::Forward without returning the convergence and scale.
     **********************************************************************/
    static void Forward(real lat, real lon, real& x, real& y) throw() {
      real gamma, k;
      Forward(lat, lon, x, y, gamma, k);
    }

    /**
     * OSGB::Reverse without returning the convergence and scale.
     **********************************************************************/
    static void Reverse(real x, real y, real& lat, real& lon) throw() {
      real gamma, k;
      Reverse(x, y, lat, lon, gamma, k);
    }

    /**
     * Convert OSGB coordinates to a grid reference.
     *
     * @param[in] x easting of point (meters).
     * @param[in] y northing of point (meters).
     * @param[in] prec precision relative to 100 km.
     * @param[out] gridref National Grid reference.
     *
     * \e prec specifies the precision of the grid reference string as follows:
     * - prec = 0 (min), 100km
     * - prec = 1, 10km
     * - prec = 2, 1km
     * - prec = 3, 100m
     * - prec = 4, 10m
     * - prec = 5, 1m
     * - prec = 6, 0.1m
     * - prec = 11 (max), 1um
     *
     * The easting must be in the range [-1000 km, 1500 km) and the northing
     * must be in the range [-500 km, 2000 km).  An exception is thrown if
     * either the easting and northing is outside these bounds.  These bounds
     * are consistent with rules for the letter designations for the grid
     * system.
     **********************************************************************/
    static void GridReference(real x, real y, int prec, std::string& gridref);

    /**
     * Convert OSGB coordinates to a grid reference.
     *
     * @param[in] gridref National Grid reference.
     * @param[out] x easting of point (meters).
     * @param[out] y northing of point (meters).
     * @param[out] prec precision relative to 100 km.
     * @param[in] centerp if true (default), return center of the grid square,
     *   else return SW (lower left) corner.
     *
     * The grid reference must be of the form: two letters (not including I)
     * followed by an even number of digits (up to 22).
     **********************************************************************/
    static void GridReference(const std::string& gridref,
                              real& x, real& y, int& prec,
                              bool centerp = true);

    /** \name Inspector functions
     **********************************************************************/
    ///@{
    /**
     * @return \e a the equatorial radius of the Airy 1830 ellipsoid (meters).
     *
     * This is 20923713 ft converted to meters using the rule 1 ft =
     * 10^(9.48401603-10) m.  (The Airy 1830 value is returned because the OSGB
     * projection is based on this ellipsoid.)
     **********************************************************************/
    static Math::real MajorRadius() throw()
    // result is about 6377563.3960320664406 m
    { return real(20923713) * std::pow(real(10), real(0.48401603L) - 1); }

    /**
     * @return \e f the inverse flattening of the Airy 1830 ellipsoid.
     *
     * For the Airy 1830 ellipsoid, \e a = 20923713 ft and \e b = 20853810 ft;
     * thus the flattening = (20923713 - 20853810)/20923713 = 7767/2324857 =
     * 1/299.32496459...  (The Airy 1830 value is returned because the OSGB
     * projection is based on this ellipsoid.)
     **********************************************************************/
    static Math::real Flattening() throw()
    { return real(20923713 - 20853810) / real(20923713); }

    /// \cond SKIP
    /**
     * <b>DEPRECATED</b>
     * @return \e r the inverse flattening of the Airy 1830 ellipsoid.
     **********************************************************************/
    static Math::real InverseFlattening() throw() { return 1/Flattening(); }
    /// \endcond

    /**
     * @return \e k0 central scale for the OSGB projection (0.9996012717).
     **********************************************************************/
    static Math::real CentralScale() throw()
    { return real(0.9996012717L); }

    /**
     * @return latitude of the origin for the OSGB projection (49 degrees).
     **********************************************************************/
    static Math::real OriginLatitude() throw() { return real(49); }

    /**
     * @return longitude of the origin for the OSGB projection (-2 degrees).
     **********************************************************************/
    static Math::real OriginLongitude() throw() { return real(-2); }

    /**
     * @return false northing the OSGB projection (-100000 meters).
     **********************************************************************/
    static Math::real FalseNorthing() throw() { return real(-100000); }

    /**
     * @return false easting the OSGB projection (400000 meters).
     **********************************************************************/
    static Math::real FalseEasting() throw() { return real(400000); }
    ///@}

  };

} // namespace GeographicLib

#if defined(_MSC_VER)
#pragma warning (pop)
#endif

#endif

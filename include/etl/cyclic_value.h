///\file

/******************************************************************************
The MIT License(MIT)

Embedded Template Library.
https://github.com/ETLCPP/etl
https://www.etlcpp.com

Copyright(c) 2014 John Wellbelove

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#ifndef ETL_CYCLIC_VALUE_INCLUDED
#define ETL_CYCLIC_VALUE_INCLUDED

///\defgroup cyclic_value cyclic_value
/// Provides a value that cycles between two limits.
/// \ingroup utilities

#include "platform.h"
#include "algorithm.h"
#include "exception.h"
#include "type_traits.h"

namespace etl
{
  //***************************************************************************
  /// Provides a value that cycles between two limits.
  //***************************************************************************
  template <typename T, T First = 0, T Last = 0, bool EtlRuntimeSpecialisation = ((First == 0) && (Last == 0))>
  class cyclic_value;

  //***************************************************************************
  /// Provides a value that cycles between two compile time limits.
  /// Supports incrementing and decrementing.
  ///\tparam T     The type of the variable.
  ///\tparam First The minimum value of the range.
  ///\tparam Last  The maximum value of the range.
  ///\ingroup cyclic_value
  //***************************************************************************
  template <typename T, T First, T Last>
  class cyclic_value<T, First, Last, false>
  {
  public:

    //*************************************************************************
    /// Default constructor.
    /// The initial value is set to the first value.
    //*************************************************************************
    ETL_CONSTEXPR cyclic_value() ETL_NOEXCEPT
      : value(First)
    {
    }

    //*************************************************************************
    /// Constructor.
    /// Set to an initial value.
    /// Clamped to the range.
    //*************************************************************************
    ETL_CONSTEXPR14 explicit cyclic_value(T initial) ETL_NOEXCEPT
    {
      set(initial);
    }

    //*************************************************************************
    /// Copy constructor.
    //*************************************************************************
    ETL_CONSTEXPR cyclic_value(const cyclic_value<T, First, Last>& other) ETL_NOEXCEPT
      : value(other.value)
    {
    }

    //*************************************************************************
    /// Assignment operator.
    //*************************************************************************
    ETL_CONSTEXPR14 cyclic_value& operator=(const cyclic_value<T, First, Last>& other) ETL_LVALUE_REF_QUALIFIER ETL_NOEXCEPT
    {
      if (this == &other)
      {
        return *this;
      }

      value = other.value;

      return *this;
    }

    //*************************************************************************
    /// Sets the value.
    /// Truncates to the First/Last range.
    ///\param value The value.
    //*************************************************************************
    ETL_CONSTEXPR14 void set(T value_) ETL_NOEXCEPT
    {
      value = etl::clamp(value_, First, Last);
    }

    //*************************************************************************
    /// Resets the value to the minimum in the range.
    //*************************************************************************
    ETL_CONSTEXPR14 void to_min() ETL_NOEXCEPT
    {
      value = First;
    }

    //*************************************************************************
    /// Resets the value to the maximum in the range.
    //*************************************************************************
    ETL_CONSTEXPR14 void to_max() ETL_NOEXCEPT
    {
      value = Last;
    }

    //*************************************************************************
    /// Advances to value by a number of steps.
    ///\param n The number of steps to advance.
    //*************************************************************************
    ETL_CONSTEXPR14 void advance(int n) ETL_NOEXCEPT
    {
      while (n > 0)
      {
        ++(*this);
        --n;
      }

      while (n < 0)
      {
        --(*this);
        ++n;
      }
    }

    //*************************************************************************
    /// Conversion operator.
    /// \return The value of the underlying type.
    //*************************************************************************
    ETL_CONSTEXPR14 operator T() ETL_NOEXCEPT
    {
      return value;
    }

    //*************************************************************************
    /// Const conversion operator.
    /// \return The value of the underlying type.
    //*************************************************************************
    ETL_CONSTEXPR operator const T() const ETL_NOEXCEPT
    {
      return value;
    }

    //*************************************************************************
    /// ++ operator.
    //*************************************************************************
    ETL_CONSTEXPR14 cyclic_value& operator++() ETL_LVALUE_REF_QUALIFIER ETL_NOEXCEPT
    {
      if (value >= Last) ETL_UNLIKELY
      {
        value = First;
      }
      else
      {
        ++value;
      }

      return *this;
    }

    //*************************************************************************
    /// ++ operator.
    //*************************************************************************
    ETL_CONSTEXPR14 cyclic_value operator++(int) ETL_NOEXCEPT
    {
      cyclic_value temp(*this);

      operator++();

      return temp;
    }

    //*************************************************************************
    /// -- operator.
    //*************************************************************************
    ETL_CONSTEXPR14 cyclic_value& operator--() ETL_LVALUE_REF_QUALIFIER ETL_NOEXCEPT
    {
      if (value <= First) ETL_UNLIKELY
      {
        value = Last;
      }
      else
      {
        --value;
      }

      return *this;
    }

    //*************************************************************************
    /// -- operator.
    //*************************************************************************
    ETL_CONSTEXPR14 cyclic_value operator--(int) ETL_NOEXCEPT
    {
      cyclic_value temp(*this);

      operator--();

      return temp;
    }

    //*************************************************************************
    /// = operator.
    //*************************************************************************
    ETL_CONSTEXPR14 cyclic_value& operator=(T t) ETL_LVALUE_REF_QUALIFIER ETL_NOEXCEPT
    {
      set(t);
      return *this;
    }

    //*************************************************************************
    /// = operator.
    //*************************************************************************
    template <const T FIRST2, const T LAST2>
    ETL_CONSTEXPR14 cyclic_value& operator=(const cyclic_value<T, FIRST2, LAST2>& other) ETL_LVALUE_REF_QUALIFIER ETL_NOEXCEPT
    {
      set(other.get());
      return *this;
    }

    //*************************************************************************
    /// Gets the value.
    //*************************************************************************
    ETL_CONSTEXPR T get() const ETL_NOEXCEPT
    {
      return value;
    }

    //*************************************************************************
    /// Gets the minimum value.
    //*************************************************************************
    static ETL_CONSTEXPR T min() ETL_NOEXCEPT
    {
      return First;
    }

    //*************************************************************************
    /// Gets the maximum value.
    //*************************************************************************
    static ETL_CONSTEXPR T max() ETL_NOEXCEPT
    {
      return Last;
    }

    //*************************************************************************
    /// Swaps the values.
    //*************************************************************************
    void swap(cyclic_value<T, First, Last>& other) ETL_NOEXCEPT
    {
      using ETL_OR_STD::swap; // Allow ADL

      swap(value, other.value);
    }

    //*************************************************************************
    /// Swaps the values.
    //*************************************************************************
    friend void swap(cyclic_value<T, First, Last>& lhs, cyclic_value<T, First, Last>& rhs) ETL_NOEXCEPT
    {
      lhs.swap(rhs);
    }

    //*************************************************************************
    /// Operator ==.
    //*************************************************************************
    friend ETL_CONSTEXPR bool operator==(const cyclic_value<T, First, Last>& lhs, const cyclic_value<T, First, Last>& rhs) ETL_NOEXCEPT
    {
      return lhs.value == rhs.value;
    }

    //*************************************************************************
    /// Operator !=.
    //*************************************************************************
    friend ETL_CONSTEXPR bool operator!=(const cyclic_value<T, First, Last>& lhs, const cyclic_value<T, First, Last>& rhs) ETL_NOEXCEPT
    {
      return !(lhs == rhs);
    }

  private:

    T value; ///< The current value.
  };

  //***************************************************************************
  /// Provides a value that cycles between two run time limits.
  /// Supports incrementing and decrementing.
  ///\tparam T     The type of the variable.
  ///\tparam First The minimum value of the range.
  ///\tparam Last  The maximum value of the range.
  ///\ingroup cyclic_value
  //***************************************************************************
  template <typename T, T First, T Last>
  class cyclic_value<T, First, Last, true>
  {
  public:

    //*************************************************************************
    /// Constructor.
    /// Sets the minimum and maximum to the template parameter values.
    /// The initial value is set to the minimum value.
    //*************************************************************************
    ETL_CONSTEXPR cyclic_value() ETL_NOEXCEPT
      : value(First)
      , first_value(First)
      , last_value(Last)
    {
    }

    //*************************************************************************
    /// Constructor.
    /// Sets the value to the minimum of the range.
    ///\param first The minimum value in the range.
    ///\param last  The maximum value in the range.
    //*************************************************************************
    ETL_CONSTEXPR cyclic_value(T first_, T last_) ETL_NOEXCEPT
      : value(first_)
      , first_value(first_)
      , last_value(last_)
    {
    }

    //*************************************************************************
    /// Constructor.
    /// Set to an initial value.
    /// Clamped to the range.
    ///\param first The minimum value in the range.
    ///\param last  The maximum value in the range.
    //*************************************************************************
    ETL_CONSTEXPR14 cyclic_value(T first_, T last_, T initial) ETL_NOEXCEPT
      : first_value(first_)
      , last_value(last_)
    {
      set(initial);
    }

    //*************************************************************************
    /// Copy constructor.
    //*************************************************************************
    ETL_CONSTEXPR cyclic_value(const cyclic_value& other) ETL_NOEXCEPT
      : value(other.value)
      , first_value(other.first_value)
      , last_value(other.last_value)
    {
    }

    //*************************************************************************
    /// Sets the range.
    /// Sets the value to the minimum of the range.
    ///\param first The minimum value in the range.
    ///\param last  The maximum value in the range.
    //*************************************************************************
    ETL_CONSTEXPR14 void set(T first_, T last_) ETL_NOEXCEPT
    {
      first_value = first_;
      last_value  = last_;
      value       = first_;
    }

    //*************************************************************************
    /// Sets the value.
    ///\param value The value.
    //*************************************************************************
    ETL_CONSTEXPR14 void set(T value_) ETL_NOEXCEPT
    {
      value = etl::clamp(value_, first_value, last_value);
    }

    //*************************************************************************
    /// Resets the value to the minimum in the range.
    //*************************************************************************
    ETL_CONSTEXPR14 void to_min() ETL_NOEXCEPT
    {
      value = first_value;
    }

    //*************************************************************************
    /// Resets the value to the maximum in the range.
    //*************************************************************************
    ETL_CONSTEXPR14 void to_max() ETL_NOEXCEPT
    {
      value = last_value;
    }

    //*************************************************************************
    /// Advances to value by a number of steps.
    ///\param n The number of steps to advance.
    //*************************************************************************
    ETL_CONSTEXPR14 void advance(int n) ETL_NOEXCEPT
    {
      while (n > 0)
      {
        ++(*this);
        --n;
      }

      while (n < 0)
      {
        --(*this);
        ++n;
      }
    }

    //*************************************************************************
    /// Conversion operator.
    /// \return The value of the underlying type.
    //*************************************************************************
    ETL_CONSTEXPR14 operator T() ETL_NOEXCEPT
    {
      return value;
    }

    //*************************************************************************
    /// Const conversion operator.
    /// \return The value of the underlying type.
    //*************************************************************************
    ETL_CONSTEXPR operator const T() const ETL_NOEXCEPT
    {
      return value;
    }

    //*************************************************************************
    /// ++ operator.
    //*************************************************************************
    ETL_CONSTEXPR14 cyclic_value& operator++() ETL_LVALUE_REF_QUALIFIER ETL_NOEXCEPT
    {
      if (value >= last_value)
      {
        value = first_value;
      }
      else
      {
        ++value;
      }

      return *this;
    }

    //*************************************************************************
    /// ++ operator.
    //*************************************************************************
    ETL_CONSTEXPR14 cyclic_value operator++(int) ETL_NOEXCEPT
    {
      cyclic_value temp(*this);

      operator++();

      return temp;
    }

    //*************************************************************************
    /// -- operator.
    //*************************************************************************
    ETL_CONSTEXPR14 cyclic_value& operator--() ETL_LVALUE_REF_QUALIFIER ETL_NOEXCEPT
    {
      if (value <= first_value)
      {
        value = last_value;
      }
      else
      {
        --value;
      }

      return *this;
    }

    //*************************************************************************
    /// -- operator.
    //*************************************************************************
    ETL_CONSTEXPR14 cyclic_value operator--(int) ETL_NOEXCEPT
    {
      cyclic_value temp(*this);

      operator--();

      return temp;
    }

    //*************************************************************************
    /// = operator.
    //*************************************************************************
    ETL_CONSTEXPR14 cyclic_value& operator=(T t) ETL_LVALUE_REF_QUALIFIER ETL_NOEXCEPT
    {
      set(t);
      return *this;
    }

    //*************************************************************************
    /// = operator.
    //*************************************************************************
    ETL_CONSTEXPR14 cyclic_value& operator=(const cyclic_value& other) ETL_LVALUE_REF_QUALIFIER ETL_NOEXCEPT
    {
      if (this == &other)
      {
        return *this;
      }

      value       = other.value;
      first_value = other.first_value;
      last_value  = other.last_value;
      return *this;
    }

    //*************************************************************************
    /// Gets the value.
    //*************************************************************************
    ETL_CONSTEXPR T get() const ETL_NOEXCEPT
    {
      return value;
    }

    //*************************************************************************
    /// Gets the minimum value.
    //*************************************************************************
    ETL_CONSTEXPR T min() const ETL_NOEXCEPT
    {
      return first_value;
    }

    //*************************************************************************
    /// Gets the maximum value.
    //*************************************************************************
    ETL_CONSTEXPR T max() const ETL_NOEXCEPT
    {
      return last_value;
    }

    //*************************************************************************
    /// Swaps the values.
    //*************************************************************************
    void swap(cyclic_value<T, First, Last>& other) ETL_NOEXCEPT
    {
      using ETL_OR_STD::swap; // Allow ADL

      swap(first_value, other.first_value);
      swap(last_value, other.last_value);
      swap(value, other.value);
    }

    //*************************************************************************
    /// Swaps the values.
    //*************************************************************************
    friend void swap(cyclic_value<T, First, Last>& lhs, cyclic_value<T, First, Last>& rhs) ETL_NOEXCEPT
    {
      lhs.swap(rhs);
    }

    //*************************************************************************
    /// Operator ==.
    //*************************************************************************
    friend ETL_CONSTEXPR bool operator==(const cyclic_value<T, First, Last>& lhs, const cyclic_value<T, First, Last>& rhs) ETL_NOEXCEPT
    {
      return (lhs.value == rhs.value) && (lhs.first_value == rhs.first_value) && (lhs.last_value == rhs.last_value);
    }

    //*************************************************************************
    /// Operator !=.
    //*************************************************************************
    friend ETL_CONSTEXPR bool operator!=(const cyclic_value<T, First, Last>& lhs, const cyclic_value<T, First, Last>& rhs) ETL_NOEXCEPT
    {
      return !(lhs == rhs);
    }

  private:

    T value;       ///< The current value.
    T first_value; ///< The first value in the range.
    T last_value;  ///< The last value in the range.
  };
} // namespace etl

#endif

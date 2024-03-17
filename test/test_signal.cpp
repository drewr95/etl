/******************************************************************************
The MIT License(MIT)

Embedded Template Library.
https://github.com/ETLCPP/etl
https://www.etlcpp.com

Copyright(c) 2021 John Wellbelove

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

#include "unit_test_framework.h"

#include "etl/signal.h"

#include <algorithm>
#include <array>
#include <sstream>
#include <string>

namespace
{
  //*************************************************************************
  /// \brief Generic output free function.
  /// \param out  Output string.
  /// \param str  Input string.
  //*************************************************************************
  void output(std::ostream& out, const std::string& str) noexcept
  {
    out << str;
  }

  //*************************************************************************
  /// \brief Free function that outputs "free"
  /// \param out  Output string.
  //*************************************************************************
  void output_free(std::ostream& out) noexcept
  {
    output(out, "free");
  }

  //*************************************************************************
  /// \brief Lmabda that outputs "lambda".
  /// \param out  Output string.
  //*************************************************************************
  auto outputLambda = [](std::ostream& out)
  { output(out, "lambda"); };

  class ExampleClass
  {
  public:
    //*************************************************************************
    /// \brief Static method that outputs "static".
    /// \param out  Output string.
    //*************************************************************************
    static void static_method(std::ostream& out) noexcept
    {
      output(out, "static");
    }

    //*************************************************************************
    /// \brief Instance method that outputs "method".
    /// \param out  Output string.
    //*************************************************************************
    void method(std::ostream& out) noexcept
    {
      output(out, "method");
    }

    //*************************************************************************
    /// \brief Functor method that outputs "functor".
    /// \param out  Output string.
    //*************************************************************************
    void operator()(std::ostream& out) const noexcept
    {
      output(out, "functor");
    }
  };

  ExampleClass example_class;

  SUITE(test_signal)
  {
    //*************************************************************************
    TEST(test_constructor)
    {
      etl::signal<void(int), 2> sig;
      CHECK_EQUAL(0U, sig.count());
      CHECK_EQUAL(2U, sig.capacity());
      CHECK_EQUAL(2U, sig.available());
      CHECK(sig.is_empty());
      CHECK(!sig.is_full());
    }

    //*************************************************************************
    TEST(test_call)
    {
      constexpr size_t                                   totalConnections = 5U;
      etl::signal<void(std::ostream&), totalConnections> sig;

      std::array<etl::scoped_connection, totalConnections> connections{{{sig.connect<output_free>()},
                                                                        {sig.connect<outputLambda>()},
                                                                        {sig.connect<&ExampleClass::static_method>()},
                                                                        {sig.connect<ExampleClass, &ExampleClass::method>(example_class)},
                                                                        {sig.connect(example_class)}}};

      std::stringstream ss;
      sig(ss);

      // expect all the signals got called
      CHECK_EQUAL(std::string{"freelambdastaticmethodfunctor"}, ss.str());
    }
  }
}  // namespace
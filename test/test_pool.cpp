/******************************************************************************
The MIT License(MIT)

Embedded Template Library.
https://github.com/ETLCPP/etl

Copyright(c) 2014 jwellbelove

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

#include <UnitTest++/UnitTest++.h>
#include "ExtraCheckMacros.h"

#include "data.h"

#include <set>
#include <vector>

#include "../pool.h"

typedef TestDataDC<std::string>  Test_Data;
typedef TestDataNDC<std::string> Test_Data2;

namespace
{
  SUITE(test_pool)
  {
    //*************************************************************************
    TEST(test_allocate)
    {
      etl::pool<Test_Data, 4> pool;
      
      Test_Data* p1;
      Test_Data* p2;
      Test_Data* p3;
      Test_Data* p4;

      CHECK_NO_THROW(p1 = pool.allocate());
      CHECK_NO_THROW(p2 = pool.allocate());
      CHECK_NO_THROW(p3 = pool.allocate());
      CHECK_NO_THROW(p4 = pool.allocate());

      CHECK(p1 != p2);
      CHECK(p1 != p3);
      CHECK(p1 != p4);
      CHECK(p2 != p3);
      CHECK(p2 != p4);
      CHECK(p3 != p4);

      CHECK_THROW(pool.allocate(), etl::pool_no_allocation);
    }

    //*************************************************************************
    TEST(test_release)
    {
      etl::pool<Test_Data, 4> pool;

      Test_Data* p1 = pool.allocate();
      Test_Data* p2 = pool.allocate();
      Test_Data* p3 = pool.allocate();
      Test_Data* p4 = pool.allocate();

      CHECK_NO_THROW(pool.release(p2));
      CHECK_NO_THROW(pool.release(*p3));
      CHECK_NO_THROW(pool.release(p1));
      CHECK_NO_THROW(pool.release(*p4));

      CHECK_EQUAL(4, pool.available());

      Test_Data not_in_pool;

      CHECK_THROW(pool.release(not_in_pool), etl::pool_object_not_in_pool);
    }

    //*************************************************************************
    TEST(test_allocate_release)
    {
      etl::pool<Test_Data, 4> pool;

      Test_Data* p1 = pool.allocate();
      Test_Data* p2 = pool.allocate();
      Test_Data* p3 = pool.allocate();
      Test_Data* p4 = pool.allocate();

      // Allocated p1, p2, p3, p4

      CHECK_EQUAL(0, pool.available());

      CHECK_NO_THROW(pool.release(p2));
      CHECK_NO_THROW(pool.release(p3));

      // Allocated p1, p4

      CHECK_EQUAL(2, pool.available());

      Test_Data* p5 = pool.allocate();
      Test_Data* p6 = pool.allocate();

      // Allocated p1, p4, p5, p6

      CHECK_EQUAL(0, pool.available());

      CHECK(p5 != p1);
      CHECK(p5 != p4);

      CHECK(p6 != p1);
      CHECK(p6 != p4);

      CHECK_NO_THROW(pool.release(p5));

      // Allocated p1, p4, p6

      CHECK_EQUAL(1, pool.available());

      Test_Data* p7 = pool.allocate();

      // Allocated p1, p4, p6, p7

      CHECK(p7 != p1);
      CHECK(p7 != p4);
      CHECK(p7 != p6);

      CHECK(pool.none_free());
    }

    //*************************************************************************
    TEST(test_available)
    {
      etl::pool<Test_Data, 4> pool;
      CHECK_EQUAL(4, pool.available());

      Test_Data* p;

      p = pool.allocate();
      CHECK_EQUAL(3, pool.available());

      p = pool.allocate();
      CHECK_EQUAL(2, pool.available());

      p = pool.allocate();
      CHECK_EQUAL(1, pool.available());

      p = pool.allocate();
      CHECK_EQUAL(0, pool.available());
    }

    //*************************************************************************
    TEST(test_none_free)
    {
      etl::pool<Test_Data, 4> pool;
      CHECK_EQUAL(4, pool.available());

      Test_Data* p;

      p = pool.allocate();
      CHECK(!pool.none_free());

      p = pool.allocate();
      CHECK(!pool.none_free());

      p = pool.allocate();
      CHECK(!pool.none_free());

      p = pool.allocate();
      CHECK(pool.none_free());
    }

    //*************************************************************************
    TEST(test_is_in_pool)
    {
      etl::pool<Test_Data, 4> pool;
      Test_Data not_in_pool;

      Test_Data* p1 = pool.allocate();

      CHECK(pool.is_in_pool(p1));
      CHECK(!pool.is_in_pool(not_in_pool));
    }

    //*************************************************************************
    TEST(test_const_iterator)
    {
      etl::pool<Test_Data2, 10> pool;

      std::set<Test_Data2>     compare = { Test_Data2("0"), Test_Data2("2"), Test_Data2("4"), Test_Data2("6"), Test_Data2("8") };
      std::set<Test_Data2>     test;
      std::vector<Test_Data2*> objects;

      // Build the set of objects.
      objects.push_back(pool.allocate(Test_Data2("9")));
      objects.push_back(pool.allocate(Test_Data2("7")));
      objects.push_back(pool.allocate(Test_Data2("8")));
      objects.push_back(pool.allocate(Test_Data2("6")));
      objects.push_back(pool.allocate(Test_Data2("5")));
      objects.push_back(pool.allocate(Test_Data2("3")));
      objects.push_back(pool.allocate(Test_Data2("4")));
      objects.push_back(pool.allocate(Test_Data2("2")));
      objects.push_back(pool.allocate(Test_Data2("0")));
      objects.push_back(pool.allocate(Test_Data2("1")));

      // Release "1", "3", "5", "7", "9".
      pool.release(objects[0]);
      pool.release(objects[1]);
      pool.release(objects[4]);
      pool.release(objects[5]);
      pool.release(objects[9]);

      // Fill the test set with what we get from the iterator.
      etl::pool<Test_Data2, 10>::const_iterator i_pool = pool.begin();

      while (i_pool != pool.end())
      {
        test.insert(*i_pool);
        ++i_pool;
      }

      // Compare the results.
      std::set<Test_Data2>::const_iterator i_compare = compare.begin();
      std::set<Test_Data2>::const_iterator i_test    = test.begin();

      CHECK_EQUAL(compare.size(), test.size());

      while ((i_compare != compare.end()) && (i_test != test.end()))
      {
        CHECK_EQUAL(*i_compare++, *i_test++);
      }
    }

    ////*************************************************************************
    //TEST(test_get_iterator)
    //{
    //  typedef etl::pool<Test_Data, 4> Pool;

    //  Pool pool;
    //  Test_Data not_in_pool;

    //  Test_Data* p1 = pool.allocate();
    //  Test_Data* p2 = pool.allocate();

    //  Pool::iterator i_data  = pool.get_iterator(*p1);
    //  Pool::iterator i_ndata = pool.get_iterator(not_in_pool);

    //  CHECK(p1 == &*i_data);
    //  CHECK(p2 != &*i_data);
    //  CHECK(pool.end() == i_ndata);
    //}

    ////*************************************************************************
    //TEST(test_get_iterator_const)
    //{
    //  typedef etl::pool<Test_Data, 4> Pool;

    //  Pool pool;
    //  const Test_Data not_in_pool;

    //  const Test_Data* p1 = pool.allocate();
    //  const Test_Data* p2 = pool.allocate();

    //  Pool::const_iterator i_data = pool.get_iterator(*p1);
    //  Pool::const_iterator i_ndata = pool.get_iterator(not_in_pool);

    //  CHECK(p1 == &*i_data);
    //  CHECK(p2 != &*i_data);
    //  CHECK(pool.end() == i_ndata);
    //}
  };
}

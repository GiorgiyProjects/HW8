#define BOOST_TEST_MODULE tests
#include "async.h"

#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( TestSuite )

    struct cout_redirect {
        cout_redirect( std::streambuf * new_buffer )
                : old( std::cout.rdbuf( new_buffer ) )
        { }

        ~cout_redirect( ) {
            std::cout.rdbuf( old );
        }

    private:
        std::streambuf * old;
    };

    BOOST_AUTO_TEST_CASE(testBulk1) {

        boost::test_tools::output_test_stream output;
        {
            cout_redirect guard(output.rdbuf());
            size_t bulk = 3;
            auto h = async::connect(bulk);
            async::receive(h, "cmd1\ncmd2\ncmd3\ncmd4\ncmd5\n", 1);
            async::disconnect(h);
        }
        BOOST_CHECK(output.is_equal ("bulk: cmd1,cmd2,cmd3\n"
                                        "bulk: cmd4,cmd5\n"
                                        ));
    }

    BOOST_AUTO_TEST_CASE(testBulk2) {

        boost::test_tools::output_test_stream output;
        {
            cout_redirect guard(output.rdbuf());
            size_t bulk = 3;
            auto h = async::connect(bulk);
            async::receive(h, "cmd1\ncmd2\n{\ncmd3\ncmd4\n}\n{\ncmd5\ncmd6\n{\ncmd7\ncmd8\n}\ncmd9\n}\n{\ncmd10\ncmd11\n", 1);
            async::disconnect(h);
        }
        BOOST_CHECK(output.is_equal ("bulk: cmd1,cmd2\n"
                                        "bulk: cmd3,cmd4\n"
                                        "bulk: cmd5,cmd6,cmd7,cmd8,cmd9\n"
        ));
    }
BOOST_AUTO_TEST_SUITE_END()
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "../libs/point.hpp"

TEST_CASE("Testing Class Point") {
    
    SECTION("Testing if point was initialized (0,0)"){
        
        Point point; 
        REQUIRE( point.getX() == 0 );
        REQUIRE( point.getY() == 0 );
    }

    SECTION("Testing negative values"){
        
        Point point;
        point.setX(-2);
        REQUIRE( point.getX() == -2 );
    }

    SECTION("Testing comparison between 2 points"){
      
        Point p1;
        Point p2;

        CHECK( p1 == p2 );

        p2.setX(2);

        CHECK( p1 != p2 );
    }
}
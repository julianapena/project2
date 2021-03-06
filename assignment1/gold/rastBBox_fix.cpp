
#include "rastBBox_fix.h"
#include "assert.h"
#include "limits.h"
#include <stdio.h>





void rastBBox_vec_fix( vector< u_Poly< long , ushort > >& polys,
						zbuff& z )
{
	int i;
	int l = polys.size();
	u_Poly< long , ushort > poly ;
	long screen_h, screen_w;

	int r_shift = 10;
	int r_val = 1024;

	screen_w = z.w * 1024;
	screen_h = z.h * 1024;

	for( i = 0 ; i < l ; i ++ ) {
		poly = polys[i];
		rastBBox_uPoly_fix( poly , z, screen_w, screen_h, 
			z.ss_i, z.ss_w_lg2 , r_shift , r_val);
	}

}

void rastBBox_uPoly_fix( u_Poly< long , ushort >& poly,
						zbuff& z, 
						long screen_w, 
						long screen_h,
						long si, //sample interval 
						int  si_lg2, //log2 of sample interval
						long r_shift, 
						long r_val )
{

  //Calculate BBox

  bool valid ; // is u_poly valid based on bbox

  long ll_x, ll_y, ur_x, ur_y ;
  long s_x , s_y ;
  long j_x , j_y ;

  int x , y , ss_x , ss_y , vert;
  fragment f ;

  rastBBox_bbox_fix( poly, ll_x, ll_y, ur_x, ur_y, 
  	z.ss_w_lg2, screen_w, screen_h, 
  	valid, r_shift, r_val  );

  //Iterate over Samples, Test if In uPoly
  for( s_x = ll_x ; s_x <= ur_x ; s_x += si ){
  	for( s_y = ll_y ; s_y <= ur_y ; s_y += si ){

  		rastBBox_jhash_jit_fix( s_x, s_y, z.ss_w_lg2, &j_x, &j_y);
  		j_x = j_x << 2;
  		j_y = j_y << 2;
  		vert = rastBBox_stest_fix( poly , s_x + j_x , s_y + j_y );

  		if( vert != -1 ){
  			x =  s_x >> r_shift;
  			y =  s_y >> r_shift;
  			ss_x = (s_x - (x << r_shift)) / si;
  			ss_y = (s_y - (y << r_shift)) / si;	
  			f.z = poly.v[vert].x[2];
  			f.c[0] = poly.v[vert].c[0];
  			f.c[1] = poly.v[vert].c[1];
  			f.c[2] = poly.v[vert].c[2];
  			f.c[3] = poly.v[vert].c[3];
  			z.process_Fragment( x , y , ss_x , ss_y ,  
  								f.z , f.c[0] , f.c[1] , 
  								f.c[2] , f.c[3] );
  		}
  	}
  }


}

static inline long floor_ss(long val, long r_shift, int& ss_w_lg2) {
  return ( val >> ( r_shift - ss_w_lg2 )) << ( r_shift - ss_w_lg2 ); 
}

void rastBBox_bbox_fix( u_Poly< long , ushort >& poly , 
						long& ll_x,
						long& ll_y,
						long& ur_x,
						long& ur_y,
			       		int&  ss_w_lg2, //log2 of subsample width
			       		long& screen_w,
			       		long& screen_h,
			       		bool& valid ,
			       		long r_shift, 
			       		long r_val )
{

  /*
  /  Function: rastBBox_bbox_fix
  /  Function Description: Determine a bounding box ll_x,ll_y,
  /   ur_x,ur_y from the micropolygon poly.  Note that this is
  /   a fixed point function.
  /
  /  Inputs: 
  /    u_Poly< long , ushort >& poly - a micropolygon.  Definition found
  /                          in helper.h. Values given in fixed point
  /    int& ss_w_lg2 - a vector indicate the level of multisample anti-aliasing 
  /        1xMSAA  : ss_w_lg2 = 0
  /        4xMSAA  : ss_w_lg2 = 1
  /        16xMSAA : ss_w_lg2 = 2
  /        64xMSAA : ss_w_lg2 = 3
  /    long& screen_w - fixed point screen width
  /    long& screen_h - fixed point screen height
  /    long r_shift - Number of Fractional Bits
  /    long r_val - 2 ^ r_shift  ( eg r_shift = 10 r_val = 1024 )
  /   
  /  Outputs:
  /    bool& valid - Is the bounding box an micropolygon valid
  /                  for example an offscreen microplygon is invalid
  /    long& ll_x - Bounding Box Lower Left X Coordinate
  /    long& ll_y - Bounding Box Lower Left Y Coordinate
  /    long& ur_x - Bounding Box Upper Right X Coordinate
  /    long& ur_y - Bounding Box Upper Right Y Coordinate
  /
  */

  /* rastBBox_bbox_fix utilized fixed point computation
  /   to determine the bounding box for the micropolygon
  /   given in poly.  Note that u_poly is specified in helper.h
  /   The bounding box values are returned via reference
  /   in the variables ll_x ll_y ur_x ur_y correpsonding
  /   to lower left coordinate and upper right coordinate.
  /   ss_w_lg2 is a log2 value related to supersampling:
  /     1xMSAA  : ss_w_lg2 = 0
  /     4xMSAA  : ss_w_lg2 = 1
  /     16xMSAA : ss_w_lg2 = 2
  /     64xMSAA : ss_w_lg2 = 3
  /   screen_w is the screen width and screen_h
  /   is the screen height.
  /   r_shift represents the total number of fractional
  /   bits utilized in the fixed point values.  r_val
  /   represents the value of those bits in fixed point.
  /   For a fixed point representation which allocates 
  /   10 bits for the fraction r_val=1024 and r_shift=10.
  /   r_shift and ss_w_lg2 are useful for rounding down
  /   to the nearest sample  ex:
  /   rounded_value = ( val >> ( r_shift - ss_w_lg2 )) << ( r_shift -
  /			       ss_w_lg2 ); 
  /   additionally it should be noted that whether a poly is 
  /   a quad or triangle can be determined by examing poly.vertices
  */

  /////
  ///// Bounding Box Function Goes Here
  ///// 

  ///// PLACE YOUR CODE HERE
  long min_x = poly.v[0].x[0];
  long max_x = poly.v[0].x[0];
  long min_y = poly.v[0].x[1];
  long max_y = poly.v[0].x[1];
  for (int i = 1; i < poly.vertices; i++) {
    min_x = ( poly.v[i].x[0] < min_x) ? poly.v[i].x[0] : min_x;
    max_x = ( poly.v[i].x[0] > max_x) ? poly.v[i].x[0] : max_x;
    min_y = ( poly.v[i].x[1] < min_y) ? poly.v[i].x[1] : min_y;
    max_y = ( poly.v[i].x[1] > max_y) ? poly.v[i].x[1] : max_y;
  }
  //ur_x = ( max_x >> ( r_shift - ss_w_lg2 )) << ( r_shift - ss_w_lg2 ); 
  ur_x = floor_ss(max_x, r_shift, ss_w_lg2);
  //ur_y = ( max_y >> ( r_shift - ss_w_lg2 )) << ( r_shift - ss_w_lg2 ); 
  ur_y = floor_ss(max_y, r_shift, ss_w_lg2);
  //ll_x = ( min_x >> ( r_shift - ss_w_lg2 )) << ( r_shift - ss_w_lg2 ); 
  ll_x = floor_ss(min_x, r_shift, ss_w_lg2);
  //ll_y = ( min_y >> ( r_shift - ss_w_lg2 )) << ( r_shift - ss_w_lg2 ); 
  ll_y = floor_ss(min_y, r_shift, ss_w_lg2);

  valid = !((ur_x < 0) 
          || (ll_x > screen_w) 
          || (ur_y < 0) 
          || (ll_y > screen_h));

  ur_x = (ur_x > screen_w) ? screen_w : ur_x;
  ur_y = (ur_y > screen_h) ? screen_h : ur_y;
  ll_x = (ll_x < 0) ? 0 : ll_x;
  ll_y = (ll_y < 0) ? 0 : ll_y;

  // //might need to always set the values even if invalid
  // if (valid) {
  //   ur_x = temp_ur_x;
  //   ur_y = temp_ur_y;
  //   ll_x = temp_ll_x;
  //   ll_y = temp_ll_y;
  // }
  
  

  /////
  ///// Bounding Box Function Goes Here
  ///// 


}



int rastBBox_stest_fix( u_Poly< long , ushort >& poly, 
	long s_x , long s_y )
{ 
  // This function determines whether the sample coordinates s_x and s_y 
  //  lie inside the micropolygon poly. Note that u_poly is specified in
  //  helper.h.  The psuedocode should be more than sufficient in explaining 
  //  the implementation.   Also, note that the fact that the values
  //  are fixed point is insignficant.

  /*
  /  Function: rastBBox_stest_fix
  /  Function Description: Does the sameple point defined by s_x and s_y 
  /                                lie inside the micropolygon
  /                                defined by poly
  /  Inputs:
  /     u_Poly< long , ushort >& poly - A micropolygon.  Definition in helper.h
  /	long s_x - The fixed point value for the sample's x coordinate
  /     long s_y - The fixed point value for the sample's y coordinate
  /
  /  Output:  
  /   Return -1   : If Miss
  /   Return  0   : If Hit
  /     note that we return (result - 1) so result can be set to 0 for
  /     miss and 1 for hit
  /
  */

  int result = 0 ; // Default to miss state
  //Shift Vertices such that sample is origin.
  //If this is a rectalge, then there is a fourth vertex. I wonder too about
  //the z dimension. There's only s_x and s_y, so the z dimension must not be 
  //importat.
  bool is_tri = poly.vertices == 3;
  long v0_x = poly.v[0].x[0] - s_x;
  long v0_y = poly.v[0].x[1] - s_y;
  long v1_x = poly.v[1].x[0] - s_x;
  long v1_y = poly.v[1].x[1] - s_y;
  long v2_x = poly.v[2].x[0] - s_x;
  long v2_y = poly.v[2].x[1] - s_y;
  //These have garbage if using a triangle
  long v3_x = poly.v[3].x[0] - s_x;
  long v3_y = poly.v[3].x[1] - s_y;

  //Distance of origin shifted edge
  long dist0 = v0_x*v1_y - v1_x*v0_y;//0-1 edge
  long dist1 = v1_x*v2_y - v2_x*v1_y;//1-2 edge
  long dist2 = (is_tri) ? v2_x*v0_y - v0_x*v2_y : v2_x*v3_y - v3_x*v2_y;//2-0 edge or 2-3 edge
  //garbage if triangle
  long dist3 = v3_x*v0_y - v0_x*v3_y; //3-0 edge

  //Test if Origin is on Right Side of Shifted Edge
  bool b0 = dist0 <= 0;
  bool b1 = dist1 < 0;
  bool b2 = dist2 <= 0;
  //garbage if triangle
  bool b3 = dist3 <= 0;

  //Triangle Min Terms with no culling
  //result = (b0 && b1 && b2) || (!b0 && !b1 && !b2)

  //Triangle Min Terms with backface culling;
  result = (is_tri) ? b0 && b1 && b2 : b0 && b1 && b2 && b3;

  return (result-1); //Return 0 if hit, otherwise return -1
}


void rastBBox_40t8_hash( uchar* arr40 , ushort* val , int shift )
{
	uchar arr32[4];
	uchar arr16[2];
	uchar arr8;

	ushort mask = 0x00ff ;
	mask = mask >> shift ;

	arr32[0] = arr40[0] ^ arr40[1] ; 
	arr32[1] = arr40[1] ^ arr40[2] ; 
	arr32[2] = arr40[2] ^ arr40[3] ; 
	arr32[3] = arr40[3] ^ arr40[4] ; 

  /*  printf( "C: %.2x %.2x %.2x %.2x\n",
      arr32[3], arr32[2] , arr32[1] , arr32[0] );*/

	arr16[0] = arr32[0] ^ arr32[2] ;
	arr16[1] = arr32[1] ^ arr32[3] ;


  // printf( "C: %.2x %.2x\n",
  //	  arr16[1], arr16[0] );

	arr8 = arr16[0] ^ arr16[1] ;

  //printf( "C: %.2x \n",
  //	  arr8 );


	mask = arr8 & mask ;
	val[0] = mask ;
}

void rastBBox_jhash_jit_fix( 
	const long& s_x,
	const long& s_y,
	const long& ss_w_lg2,
	long* jitter_x,
	long* jitter_y)
{

	long  x = s_x >> 4 ;
	long  y = s_y >> 4 ; 
	uchar arr40_1[5] ;
	uchar arr40_2[5] ;

	long* arr40_1_ptr = (long*)arr40_1;
	long* arr40_2_ptr = (long*)arr40_2;

	ushort val_x[1] ; 
	ushort val_y[1] ; 

	*arr40_1_ptr = ( y << 20 ) | x ; 
	*arr40_2_ptr = ( x << 20 ) | y ; 

	rastBBox_40t8_hash( arr40_1 , val_x , ss_w_lg2 );
	rastBBox_40t8_hash( arr40_2 , val_y , ss_w_lg2 );

	*jitter_x = (long)( val_x[0] );
	*jitter_y = (long)( val_y[0] );

}



//
//  Structs.h
//  jswf
//
//  Created by Alexander Rath on 17.01.15.
//  Copyright (c) 2015 Alexander Rath. All rights reserved.
//

/**
 * @file
 * Provides basic structures.
 */

#ifndef jswf_Structs_h
#define jswf_Structs_h

namespace jswf {
  namespace flash {
    /**
     * Represents <tt>RECT</tt> records.
     */
    struct Rect {
      sb_t x0, //!< The low x-coordinate.
           y0; //!< The low y-coordinate.
      sb_t x1, //!< The high x-coordinate.
           y1; //!< The high y-coordinate.
    };
    
    /**
     * Represents <tt>RGB</tt>, <tt>RGBA</tt> and <tt>ARGB</tt> color records.
     */
    struct RGBA {
      uint8_t r, //!< Red channel.
              g, //!< Green channel.
              b, //!< Blue channel.
              a; //!< Alpha channel.
    };
    
    /**
     * Represents <tt>CXFORM</tt> and <tt>CXFORMWITHALPHA</tt> records.
     * For each channel \f$v\f$ from \f${r,g,b,a}\f$, the transformed value \f$v'\f$ can be computed as follows:\n
     * \f$v' = max(0, min((v \cdot vM \cdot \frac{1}{256}) + vA, 255))\f$
     */
    struct ColorTransform {
      // r,g,b,a = color channels
      // M = multiplication, A = addition
      
      sb_t rM = 256, rA = 0;
      sb_t gM = 256, gA = 0;
      sb_t bM = 256, bA = 0;
      sb_t aM = 256, aA = 0;
    };
    
    /**
     * Represents <tt>MATRIX</tt> records.
     * \f[
         \begin{bmatrix}
           s_x & r_1 & t_x \\
           r_0 & s_y & t_y \\
           0 & 0 & 1
         \end{bmatrix}
       \f]
     * @todo These structures are in the wrong file.
     */
    struct Matrix {
      fb_t sx = 1, //!< Scale of the x-coordinate
           sy = 1; //!< Scale of the y-coordinate
      fb_t r0 = 0, //!< Rotation-skew (y => x')
           r1 = 0; //!< Rotation-skew (x => y')
      sb_t tx = 0, //!< Translation of the x-coordinate
           ty = 0; //!< Translation of the y-coordinate
      
      /**
       * Calculates the product of two instances of Matrix.
       * given \f$a, b, c \in \mathbb{R}^{3 \times 3}\f$\n
       * \f[a \left( b \begin{pmatrix}x\\y\\1\end{pmatrix} \right) = c \begin{pmatrix}x\\y\\1\end{pmatrix}\f]\n
         \f[
           c = a \times b = \begin{bmatrix}
             s_{x_a}s_{x_b} + r_{1_a}r_{0_b} & s_{x_a}r_{1_b} + r_{1_a}s_{y_b} & s_{x_a}t_{x_b} + r_{1_a}t_{y_b} + t_{x_a} \\
             s_{y_a}r_{0_b} + r_{0_a}s_{x_b} & s_{y_a}s_{y_b} + r_{0_a}r_{1_b} & s_{y_a}t_{y_b} + r_{0_a}t_{x_b} + t_{y_a} \\
             0 & 0 & 1
           \end{bmatrix}
         \f]
       * @param [out] result A reference to the Matrix to store the result in.
       * @param [in] a,b The operands.
       */
      static void product(Matrix &result, const Matrix &a, const Matrix &b) {
        result.sx = a.sx * b.sx + a.r0 * b.r1;
        result.sy = a.sy * b.sy + a.r1 * b.r0;
        
        result.r0 = a.r0 * b.sy + a.sx * b.r0;
        result.r1 = a.r1 * b.sx + a.sy * b.r1;
        
        result.tx = a.tx * b.sx + a.ty * b.r1 + b.tx;
        result.ty = a.ty * b.sy + a.tx * b.r0 + b.ty;
      }
      
      /**
       * Transforms a point using this Matrix.
       * \f[
           \begin{bmatrix}
             s_x & r_1 & t_x \\
             r_0 & s_y & t_y \\
             0 & 0 & 1
           \end{bmatrix}
           \begin{pmatrix}x\\y\\1\end{pmatrix}
           =
           \begin{pmatrix}
             x \cdot s_x + y \cdot r_1 + t_x \\
             y \cdot s_y + x \cdot r_0 + t_y \\
             1
           \end{pmatrix}
         \f]
       */
      inline void transform(sb_t &x, sb_t &y) const {
        sb_t t = x; // temp.
        x = x * sx + y * r1 + tx;
        y = y * sy + t * r0 + ty;
      }
      
      /**
       * Calculates the inverse of this Matrix.
       * \f[
           \begin{bmatrix}
             s_x & r_1 & t_x \\
             r_0 & s_y & t_y \\
             0 & 0 & 1
           \end{bmatrix}^{-1}
           =
           \frac{1}{s_x s_y - r_0 r_1}
           \begin{bmatrix}
             s_y & -r_1 & t_y r_1 - t_x s_y \\
             -r_0 & s_x & t_x r_0 - t_y s_x \\
             0 & 0 & 1
           \end{bmatrix}
         \f]
       * @param [out] result A reference to the Matrix to store the result in.
       * @return <tt>true</tt> if the operation was successful, <tt>false</tt> if the Matrix cannot be inversed
       * @warning This will fail if the determinant of this Matrix (\f$s_x s_y - r_0 r_1\f$) equals zero.
       *          In that case the given Matrix cannot be inversed
       *          and no write to result occurs.
       */
      bool inverse(Matrix &result) const {
        double det = sx * sy - r0 * r1;
        if(det == 0) return false;
        
        double f = 1 / det;
        
        result.sx = f * sy;
        result.r1 = - f * r1;
        result.tx = f * (ty * r1 - tx * sy);
        
        result.r0 = - f * r0;
        result.sy = f * sx;
        result.ty = f * (tx * r0 - ty * sx);
        
        return true;
      }
    };
  }
}

#endif
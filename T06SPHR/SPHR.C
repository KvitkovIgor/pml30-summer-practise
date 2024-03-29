/* FILE NAME: SPHR.C
 * PROGRAMMER: IK3
 * DATE: 02.06.2016
 * PURPOSE: WinAPI windowed applictaion sample
 */
#include "sphr.h"
#define M 300
#define N 600

typedef double DBL;

typedef struct
{          
  DBL X, Y, Z;
} VEC;

/* Image representation type */
typedef struct
{
  HBITMAP hBmGlobe;
  DWORD *Bits;
  INT W, H;
} IMG;


/* Dot product of two vectors function.
 * ARGUMENTS:
 *   - vectors to be dot producted:
 *       VEC A, B;
 * RETURNS:
 *   (DBL) dot product value.
 */
DBL VecDotVec( VEC A, VEC B )
{
  return A.X * B.X + A.Y * B.Y + A.Z * B.Z;
} /* End of 'VecDotVec' function */

/* Vector set by components function.
 * ARGUMENTS:
 *   - vector components:
 *       DBL A, B, C;
 * RETURNS:
 *   (VEC) constructed vector.
 */
VEC VecSet( DBL A, DBL B, DBL C )
{
  VEC r = {A, B, C};

  return r;
} /* End of 'VecSet' function */

/* Vector normalization function.
 * ARGUMENTS:
 *   - vector to be normalize:
 *       VEC V;
 * RETURNS:
 *   (VEC) normalized vector value.
 */
VEC VecNormalize( VEC V )
{
  DBL len = VecDotVec(V, V);

  if (len != 1 && len != 0)
    len = sqrt(len), V.X /= len, V.Y /= len, V.Z /= len;
  return V;
} /* End of 'VecNormalize' function */

/* Rotate vector function.
 * ARGUMENTS:
 *   - vector to be rotated:
 *       VEC P;
 *   - vector rotated around:
 *       VEC A;
 *   - rotation angle in degree:
 *       DBL Angle;
 * RETURNS:
 *   (VEC) rotated vector value.
 */
VEC Rotate( VEC P, VEC A, DBL Angle )
{
  DBL si, co;

  A = VecNormalize(A);

  Angle *= pi / 30;
  si = sin(Angle);
  co = cos(Angle);

  return VecSet(P.X * (co + A.X * A.X * (1 - co)) +
                P.Y * (A.Y * A.X * (1 - co) + A.Z * si) +
                P.Z * (A.Z * A.X * (1 - co) - A.Y * si),
                P.X * (A.X * A.Y * (1 - co) - A.Z * si) +
                P.Y * (co + A.Y * A.Y * (1 - co)) + 
                P.Z * (A.Z * A.Y * (1 - co) + A.X * si),
                P.X * (A.X * A.Z * (1 - co) + A.Y * si) +
                P.Y * (A.Y * A.Z * (1 - co) - A.X * si) + 
                P.Z * (co + A.Z * A.Z * (1 - co)));
} /* End of 'Rotate' function */
/***
 * Sphere handle functions
 ***/

IMG Globe;

/* Load sphere texture function.
 * ARGUMENTS: None.
 * RETURNS: None.
 */
VOID LoadSphere( VOID )
{
  HDC hDC = GetDC(NULL), hMemDC, hMemDC1;
  HBITMAP hBm;
  BITMAP bm;
  BITMAPINFOHEADER bmih;

  /* Load image from file */
  hMemDC = CreateCompatibleDC(hDC);
  hBm = LoadImage(NULL, "B.BMP", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
  GetObject(hBm, sizeof(bm), &bm);
  Globe.W = bm.bmWidth;
  Globe.H = bm.bmHeight;
  SelectObject(hMemDC, hBm);

  /* Fill DIB info */
  memset(&bmih, 0, sizeof(bmih));
  bmih.biSize = sizeof(BITMAPINFOHEADER);
  bmih.biBitCount = 32;                   /* BGRx - DWORD */
  bmih.biPlanes = 1;                      /* always */
  bmih.biCompression = BI_RGB;            /* 0 - no compression */
  bmih.biWidth = bm.bmWidth;              /* width */
  bmih.biHeight = -bm.bmHeight;           /* height + first coded row is 0 */
  bmih.biSizeImage = bm.bmWidth * bm.bmHeight * 4;  /* image size in bytes */

  Globe.hBmGlobe = CreateDIBSection(NULL, (BITMAPINFO *)&bmih, DIB_RGB_COLORS,
                 (VOID **)&Globe.Bits, NULL, 0);
  hMemDC1 = CreateCompatibleDC(hDC);
  SelectObject(hMemDC1, Globe.hBmGlobe);

  BitBlt(hMemDC1, 0, 0, Globe.W, Globe.H, hMemDC, 0, 0, SRCCOPY);

  DeleteDC(hMemDC);
  DeleteDC(hMemDC1);
  DeleteObject(hBm);

  ReleaseDC(NULL, hDC);
} /* End of 'LoadSphere' function */
VOID DrawQuad( HDC hDC, POINT P0, POINT P1, POINT P2, POINT P3, DWORD Color )
{
  INT s =
    (P0.x - P1.x) * (P0.y + P1.y) +
    (P1.x - P2.x) * (P1.y + P2.y) +
    (P2.x - P3.x) * (P2.y + P3.y) +
    (P3.x - P0.x) * (P3.y + P0.y);

  if (s > 0)
  {
    POINT pts[4];
    
    pts[0] = P0;
    pts[1] = P1;
    pts[2] = P2;
    pts[3] = P3;

    Polygon(hDC, pts, 4);
  }
} /* End of 'DrawQuad' function */
VOID DrawSphere( HDC hDC, INT Xc, INT Yc, INT R )
{
  INT i, j, x, y;
  DBL theta, phi;
  static POINT pts[M][N];
  POINT p0, p1, p2, p3;
  static VEC G[M][N], A = 
  {
    1, 1, 1
  };

  for (i = 0; i < M; i++)
  {
    theta = i * pi / (M - 1);
    for (j = 0; j < N; j++)
    {
      phi = j * 2 * pi / (N - 1) + clock() / 1000.0;
      G[i][j].X =  R * sin(theta) * cos(phi);
      G[i][j].Y =  R * sin(theta) * sin(phi);
      G[i][j].Z =  R * cos(theta); 
    
      G[i][j] = Rotate(G[i][j], A, 30 * sin(clock() / 1000.0));
    }                          
  }
  /*
  for (i = 0; i < M; i++)
  {
    x = Xc + G[i][0].X;
    y = Yc - G[i][0].Z;
    MoveToEx(hDC, x, y, NULL);
    for (j = 1; j < N; j++)
    {
      x = Xc + G[i][j].X;
      y = Yc - G[i][j].Z; 
     
      LineTo(hDC, x, y);
    }
  }

  for (j = 0; j < N; j++)
  {
    x = Xc + G[0][j].X;
    y = Yc - G[0][j].Z;
    MoveToEx(hDC, x, y, NULL);
    for (i = 1; i < M; i++)
    {
      x = Xc + G[i][j].X;
      y = Yc - G[i][j].Z; 
      
      LineTo(hDC, x, y);
    }
   }
   */
  for (i = 0; i < M; i++)
    for (j = 0; j < N; j++)
    {
      pts[i][j].x = Xc + G[i][j].X;
      pts[i][j].y = Yc - G[i][j].Z;
    }

  LoadSphere();
  for (i = 0; i < M - 1; i++)
  {
    for (j = 0; j < N - 1; j++)
    {
      INT img_x, img_y;
      COLORREF c;
      BYTE r, g, b;

      img_x = j * (Globe.W - 1) / (N - 1);
      img_y = i * (Globe.H - 1) / (M - 1);

      c = Globe.Bits[img_x + img_y * Globe.W]; 
      r = GetRValue(c);
      g = GetGValue(c);
      b = GetBValue(c);
      c = RGB(b, g, r);
    
      SelectObject(hDC, GetStockObject(DC_PEN));
      SelectObject(hDC, GetStockObject(DC_BRUSH));

      SetDCPenColor(hDC, c);                  
      SetDCBrushColor(hDC, c);                                                                
      p0 = pts[i][j];                                                                             
      p3 = pts[i + 1][j];
      p1 = pts[i][j + 1];
      p2 = pts[i + 1][j + 1];
      if (G[i][j].Y < 0)
        DrawQuad(hDC, p0, p1, p2, p3, 0); 
    }
  }

  SetDCPenColor(hDC, RGB(255, 255, 255));
  SetDCBrushColor(hDC, RGB(255, 255, 255));
}
/* Draw quadrilateral function.
 * ARGUMENTS:
 *   - drawing device context:
 *       HDC hDC;
 *   - corner points:
 *       POINT P0, P1, P2, P3;
 *   - color:
 *       DWORD Color;
 * RETURNS: None.
 */







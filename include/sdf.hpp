
namespace renderer {

// A Note on “Fast Raster Scan Distance Propagation on the Discrete Rectangular Lattice”
// http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.399.4920&rep=rep1&type=pdf
// http://www.imm.dtu.dk/~jmca/02501/lectures/02501_objects.pdf
// 8SSEDT = 8 PointS Sequential Euclidean Distance Transform

namespace sdf {
    struct Point
    {
        int dx, dy;

        int DistSq() const { return dx*dx + dy*dy; }
    };

    class Grid {
    public:
        Grid(size_t w, size_t h) {
            width = w;
            height = h;
            data = new Point*[h];
            for(int y = 0; y < h; y++){
                data[y] = new Point[w];
            }
        }
        ~Grid() {
            for(int y = 0; y < height; y++){
                delete[] data[y];
            }
            delete[] data;
        }
        Point** data;
    private:
        size_t width;
        size_t height;
    };


    class SDFBuilder {
    private:
        size_t width;
        size_t height;
        Point inside = { 0, 0 };
        Point empty = { 9999, 9999 };
        Grid grid1;
        Grid grid2;
    public:
        SDFBuilder(size_t w, size_t h):
            width(w),
            height(h),
            grid1(w, h),
            grid2(w, h)
        {
            
        }

        Point Get(Grid &g, int x, int y) {
            // OPTIMIZATION: you can skip the edge check code if you make your grid 
            // have a 1-pixel gutter.
            if ( x >= 0 && y >= 0 && x < width && y < height)
                return g.data[y][x];
            else
                return empty;
        }

        void Put(Grid &g, int x, int y, const Point &p)
        {
            g.data[y][x] = p;
        }

        void Compare(Grid &g, Point &p, int x, int y, int offsetx, int offsety)
        {
            Point other = Get(g, x+offsetx, y+offsety);
            other.dx += offsetx;
            other.dy += offsety;
            if (other.DistSq() < p.DistSq())
                p = other;
        }

        template<typename Input, typename Output>
        void buildSDF(Input inputFunc, Output outputFunc) {
            for( int y = 0; y < height; y++ ) {
                for ( int x = 0; x < width; x++ ) {
                    float g = inputFunc(x, y);
                    if (g < 0.5f) {
                        Put(grid1, x, y, inside);
                        Put(grid2, x, y, empty);
                    } else {
                        Put(grid2, x, y, inside);
                        Put(grid1, x, y, empty);
                    }
                }
            }
            for(int y = 0; y < height; y++){
                for(int x = 0; x < width; x++){
                    printf("%d\t", Get(grid1, x, y));
                }
                printf("\n");
            }
            // Generate the SDF.
            GenerateSDF( grid1 );
            for(int y = 0; y < height; y++){
                for(int x = 0; x < width; x++){
                    printf("%d\t", Get(grid1, x, y));
                }
                printf("\n");
            }
            GenerateSDF( grid2 );
            
            for( int y=0;y<height;y++ ) {
                for ( int x=0;x<width;x++ ) {
                    // Calculate the actual distance from the dx/dy
                    int dist1 = (int)( sqrt( (double)Get( grid1, x, y ).DistSq() ) );
                    int dist2 = (int)( sqrt( (double)Get( grid2, x, y ).DistSq() ) );
                    // printf("dist1, dist2: %d,%d\n", dist1, dist2);
                    int dist = dist1 - dist2;
                    outputFunc(x, y, dist);
                }
            }    
        }

        void GenerateSDF(Grid &g)
        {
            // Pass 0
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    Point p = Get( g, x, y );
                    Compare( g, p, x, y, -1,  0 );
                    Compare( g, p, x, y,  0, -1 );
                    Compare( g, p, x, y, -1, -1 );
                    Compare( g, p, x, y,  1, -1 );
                    Put( g, x, y, p );
                }

                for (int x = width - 1; x >= 0; x--) {
                    Point p = Get(g, x, y);
                    Compare(g, p, x, y, 1, 0);
                    Put(g, x, y, p);
                }
            }

            // Pass 1
            for (int y = height-1; y >= 0; y--) {
                for (int x = width-1; x >= 0; x--) {
                    Point p = Get(g, x, y);
                    Compare(g, p, x, y,  1,  0);
                    Compare(g, p, x, y,  0,  1);
                    Compare(g, p, x, y, -1,  1);
                    Compare(g, p, x, y,  1,  1);
                    Put(g, x, y, p);
                }
                for (int x = 0; x < width; x++) {
                    Point p = Get(g, x, y);
                    Compare(g, p, x, y, -1, 0);
                    Put(g, x, y, p);
                }
            }
        }
    };
}
}

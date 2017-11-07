import glob
from PIL import Image
from math import floor
palette = [[0,0,0]];

def transform( pixel ):
    return [ 20 * ( pixel[ 0 ] / 20), 20 * ( pixel[ 1 ] / 20), 20 * ( pixel[ 2 ] / 20 ) ]

def add_to_palette( filename ):
    imgFile = Image.open( filename )
    img = imgFile.load()
    for y in range( 0, imgFile.height ):
        for x in range( 0, imgFile.width ):
            pixel = img[ x, y ]
            adjusted = transform( pixel )

            if pixel[ 3 ] < 254:
                adjusted = [ 255, 0, 255 ]
                
            if palette.count( adjusted ) == 0:
                palette.append( adjusted )

for filename in glob.glob('res/*.png'):
    add_to_palette( filename )

palette.sort()

print len( palette )

for pixel in palette:
    print str(pixel[ 0 ] ) + "\t" + str(pixel[ 1 ] ) + "\t" + str(pixel[ 2 ] )

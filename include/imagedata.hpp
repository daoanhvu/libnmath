#ifndef _IMAGE_DATA_H
#define _IMAGE_DATA_H

#include <iostream>
#include <vector>

/**
 *   This class is used to support OpenGL to generate vertices for rendering   
 */

namespace nmath {
    template <typename T>
	class ImageData {
	private:
        unsigned int vertexCount;
        short dimension;
        // This is the offset of normal vector in a data tupe
        short normalOffset;
        std::vector<short> indices;
        std::vector<T> data;
        /**
		 * rowInfo holds the number of element on each row
		 * rowCount is the number of row
		*/
		std::vector<int> rowInfo;
	public:
		ImageData();
        ImageData(unsigned int vcount, short dim, short noffs, const int* rows, int rowCount);
		~ImageData();

        T operator[] (int idx) {
            return data[idx];
        }

        void generateIndices();

        // This is for testing purpose
        void printIndices(std::ostream &out) {
            out << std::endl;
            for(auto i=0; i<indices.size(); i++) {
                out << " " << indices[i];
            }
            out << std::endl;
        }
	};

    template <typename T>
    ImageData<T>::ImageData():vertexCount(0), dimension(0), normalOffset(-1) {
    }

    template <typename T>
    ImageData<T>::ImageData(unsigned int vcount, short dim, short noffs, const int* rows, int rowCount)
        :vertexCount(vcount), dimension(dim), normalOffset(noffs) {
        for(auto i=0; i<rowCount; i++) {
            rowInfo.push_back(rows[i]);
        }
    }

    template <typename T>
    ImageData<T>::~ImageData() {
    }

    template <typename T>
    void ImageData<T>::generateIndices() {
        short count = 0;
        indices.clear();
        int rowCount = rowInfo.size();
        for(int i=0; i<rowCount-1; i++) {
            auto vertexCountLine = rowInfo[i];
            auto nexRowCount = rowInfo[i + 1];
            for(int j=0; j<vertexCountLine; j++) {
                if(j == vertexCountLine - 1) {
                    indices.push_back(count);
                    if(j>=nexRowCount) {
                        indices.push_back(count);
                    } else {
                        // add the vertex right below if any
                        indices.push_back(count + vertexCountLine);
                        if( (i == rowCount-2) && (j == nexRowCount - 1)) {
                            indices.push_back(count + vertexCountLine);
                        }
                    }
                    int e = count + vertexCountLine + 1;
                    int e1 = count + nexRowCount + 1;
                    for(auto k=e; k<e1; k++) {
                        indices.push_back(k);
                        // if(i < rowCount-2) {
                            indices.push_back(k);
                            indices.push_back(count);
                            indices.push_back(count);
                            indices.push_back(k);
                            if(k == e1-1) {
                                indices.push_back(k);
                            }
                        // }
                    }
                } else {
                    if(i>0 && j==0) {
                        //Degenerated case
                        indices.push_back(count);
                    }
                    indices.push_back(count);
                    // add the vertex right below if any
                    if(j < nexRowCount) {
                        indices.push_back(count + vertexCountLine);
                    }
                }
                count++;
            }
        }
    }
}

#endif

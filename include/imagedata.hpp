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
        T *data;
        unsigned int dataSize;
		unsigned int loggedSize;
        /**
		 * rowInfo holds the number of element on each row
		 * rowCount is the number of row
		*/
		std::vector<int> rowInfo;
	public:
		ImageData();
        ImageData(unsigned int vcount, short noffs, const int* rows, int rowCount);
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
    ImageData<T>::ImageData():vertexCount(0), dimension(0), normalOffset(-1), data(NULL) {
    }

    template <typename T>
    ImageData<T>::ImageData(unsigned int vcount, short noffs, const int* rows, int rowCount)
        :vertexCount(vcount), normalOffset(noffs) {
        for(auto i=0; i<rowCount; i++) {
            rowInfo.push_back(rows[i]);
        }
    }

    template <typename T>
    ImageData<T>::~ImageData() {

    }

    template <typename T>
    void ImageData<T>::generateIndices() {
        auto rowCount = rowInfo.size();
        short count = 0;
        this->indices.clear();
        for(auto i=0; i<rowCount-1; i++) {
            auto vertexCountLine = rowInfo[i];
            auto nexRowCount = rowInfo[i + 1];

            for(auto j=0; j<vertexCountLine; j++) {
                indices.push_back(count);
                if(i>0 && j==0) { //first element of rows that are not the first row
					indices.push_back(count);
				}

                if(j == (vertexCountLine-1) ) {
                    if(i < rowCount - 2) {
                        if( (count + vertexCountLine) < vertexCount && j<nexRowCount){
							//neu co 1 phan tu ngay ben duoi
							indices.push_back(count + vertexCountLine);
							indices.push_back(count);
						}
						indices.push_back(count);
                    } else if(( (count + vertexCountLine) < vertexCount) && (j < nexRowCount)) {
                        auto k = count + vertexCountLine;
						while(k < vertexCount) {
							indices.push_back(k);
							k++;
							if(k < vertexCount)
								indices.push_back(k);
							k++;
							if(k < vertexCount)
								indices.push_back(count);
						}
                    }
                } else {
                    //neu ngay ben duoc co mot vertex nua
					if( (count + vertexCountLine) < vertexCount && j<nexRowCount) {
						indices.push_back(count + vertexCountLine);
					} else { //neu khong thi add vertex cuoi cung cua dong duoi
						auto diff = j - nexRowCount + 1;
						indices.push_back(count + vertexCountLine - diff);
					}
                }
                count++;
            }
        }
    }
}

#endif

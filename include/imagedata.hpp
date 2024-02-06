#ifndef _IMAGE_DATA_H
#define _IMAGE_DATA_H

#include <iostream>
#include <vector>


void intColorToFloats(int color, float colors[4]) {
    const float MAX_COLOR_VALUE = 255.0f;
    colors[0] = (color & 0xFF) / MAX_COLOR_VALUE;
    colors[1] = ((color >> 8) & 0xFF) / MAX_COLOR_VALUE;
    colors[2] = ((color >> 16) & 0xFF) / MAX_COLOR_VALUE;
    colors[3] = ((color >> 24) & 0xFF) / MAX_COLOR_VALUE;
}
/**
 *   This class is used to support OpenGL to generate vertices for rendering   
 */
namespace nmath {
    template <typename T>
	class ImageData {
	private:
        unsigned int vertexCount;
        short dimension;
        // This is the offset of normal vector in a data tube
        short normalOffset;
        // This is vertices data
        std::vector<T> data;
        /**
		 * rowInfo holds the number of element on each row
		 * rowCount is the number of row
		*/
		std::vector<int> rowInfo;
	public:
		ImageData();
        ImageData(short dim);
        ImageData(unsigned int vcount, short dim, short noffs, const int* rows, int rowCount);
		virtual ~ImageData();

        T operator[] (int idx) {
            return data[idx];
        }

        void addData(T val) {
            data.push_back(val);

        }

        void calculateVertexCount() {
            this->vertexCount = data.size() / this->dimension;
        }

        unsigned int getVertexCount() const {
            return vertexCount;
        }

        unsigned int vertexListSize() {
            return data.size();
        }

        unsigned int copyDataTo(T* anArray) {
            T* source = &data[0];
            unsigned int size = data.size();
            memcpy(anArray, source, size * sizeof(T));
            return size;
        }

        unsigned int copyDataWithColorTo(int color, T* anArray) const {
            const T* source = &data[0];
            float colors[4];
            unsigned int  float4Size = 4 * sizeof(float);
            intColorToFloats(color, colors);
            unsigned int size = data.size();
            int stride = size / vertexCount;
            unsigned int  strideTSize = stride * sizeof(T);
            for (auto i=0; i<vertexCount; i++) {
                auto index = i * stride;
                memcpy(anArray + index, source + index, strideTSize);
                memcpy(anArray + index + stride, colors, float4Size);
            }
            return vertexCount * (stride + 4);
        }

        T* getData() {
            return data.data();
        }

        void addRow(int elementOnNewRow) {
            rowInfo.push_back(elementOnNewRow);
        }

        int getRowAt(int idx) { return rowInfo[idx]; }

        unsigned int getRowCount() { return rowInfo.size(); }
        
        unsigned short* generateIndicesForTriangleStrip();

        unsigned short* generateIndices(unsigned int &len);

        void setNormalOffset(int _normalOffset) {
            this->normalOffset = _normalOffset;
        }

        // std::vector<unsigned short> getListIndices() const {
        //     return indices;
        // }

        // unsigned short* getIndices() const {
        //     return indices.data();
        // }

        // unsigned int indicesSize() const {
        //     return indices.size();
        // }

        // This is for testing purpose
        void printIndices(std::ostream &out) {
            // out << std::endl;
            // for(auto i=0; i<indices.size(); i++) {
            //     out << " " << indices[i];
            // }
            // out << std::endl;
        }
	};

    template <typename T>
    ImageData<T>::ImageData():vertexCount(0), dimension(0), normalOffset(-1) {
    }

    template <typename T>
    ImageData<T>::ImageData(short dim):vertexCount(0), dimension(dim), normalOffset(-1) {
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
    unsigned short* ImageData<T>::generateIndicesForTriangleStrip() {
		// Now build the index data
        int rows = rowInfo.size();
        int cols = rowInfo[0];
		int numStripsRequired = rows - 1;
		int numDegensRequired = 2 * (numStripsRequired - 1);
		int verticesPerStrip = 2 * cols;
		// short[] heightMapIndexData = new short[(verticesPerStrip * numStripsRequired)
		//         + numDegensRequired];
		std::vector<unsigned short> indices;  
		for (auto y = 0; y<rows-1; y++) {
			if (y > 0) {
		        // Degenerate begin: repeat first vertex
				indices.push_back((unsigned short)(y * cols));
		    }
		 
		    for (int x = 0; x < cols; x++) {
		        // One part of the strip
		    	indices.push_back((unsigned short) ((y * cols) + x));
		        indices.push_back((unsigned short) (((y + 1) * cols) + x));
		    }
		 
		    if (y < rows - 2) {
		        // Degenerate end: repeat last vertex
		    	indices.push_back((unsigned short) (((y + 1) * cols) + (cols - 1)));
		    }
		}

        unsigned int indSize = sizeof(unsigned short) * indices.size();
        unsigned short* results = new unsigned short[indices.size()];
        memcpy(results, indices.data(), indSize);
        return results;
	}

    template <typename T>
    unsigned short* ImageData<T>::generateIndices(unsigned int &len) {
        unsigned short count = 0;
        std::vector<unsigned short> indices;
        int rowCount = (int)rowInfo.size();
        int orientation = 0;
        unsigned int count_pass = 0;
        for(int i=0; i<rowCount-1; i++) {
            count_pass += (i <= 0) ? 0 :  rowInfo[i-1];
            auto vertexCountLine = rowInfo[i];
            auto nexRowCount = rowInfo[i + 1];
            int j = 0;
            while(j<vertexCountLine) {
                // If we are at the last position of row i
                if(j == vertexCountLine - 1) {
                    indices.push_back(count);
                    // If the number of vertex on row i is greater than the number of vertex on the next row
                    if(j>=nexRowCount) {
                        indices.push_back(count);
                    } else {
                        // add the vertex right below
                        indices.push_back(count + vertexCountLine);

                        // If j is the last position on the next row so 
                        // this is a degeneration
                        if(j == nexRowCount - 1) {
                            // degenerate
                            indices.push_back(count + vertexCountLine);
                        }

                        unsigned short e = count + vertexCountLine + 1;
                        unsigned short e1 = count + nexRowCount + 1;
                        for(auto k=e; k<e1; k++) {
                            indices.push_back(k);
                            if(k == e1-1) {
                                indices.push_back(k);
                            } else {
                                indices.push_back(count);
                            }

                            // indices.push_back(k);
                            // if(i < rowCount-2) {
                                // indices.push_back(k);
                                // indices.push_back(count);
                                // indices.push_back(count);
                                // indices.push_back(k);
                                // if(k == e1-1) {
                                //     indices.push_back(k);
                                // }
                            // }
                        }
                    }
                } else {
                    // If this is not the first row and 
                    // we are at the first element of it
                    if(i>0 && j==0) {
                        //Degenerated case
                        indices.push_back(count);
                    }
                    indices.push_back(count);
                    // add the vertex right below if any
                    if(j < nexRowCount) {
                        indices.push_back(count + vertexCountLine);
                    } else {
                        int lastIndexOnNextRow = count + vertexCountLine - 1;
                        int e = count + vertexCountLine + 1;
                        int e1 = count + nexRowCount + 1;
                        for(auto k=j; k<(vertexCountLine-1); k++) {
                            indices.push_back(++count);
                            if(k==vertexCountLine-2) {
                                indices.push_back(count);
                            } else {
                                indices.push_back(lastIndexOnNextRow);
                            }
                        }
                        j = vertexCountLine - 1;
                    }
                }
                count++;
                j++;
            }
        }

        //post processing for tail degenerate triangles
        if(indices[indices.size()-1] == indices[indices.size()-2]) {
            if(indices[indices.size()-3] == indices[indices.size()-4]) {
                indices.pop_back();
                indices.pop_back();
                indices.pop_back();
            } else {
                indices.pop_back();
            }
        }

        unsigned int indSize = sizeof(unsigned short) * indices.size();
        unsigned short* results = new unsigned short[indices.size()];
        memcpy(results, indices.data(), indSize);

        return results;
    }
}

#endif

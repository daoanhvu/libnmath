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
        // This is the offset of normal vector in a data tube
        short normalOffset;
        std::vector<unsigned short> indices;

        // This is vertices data
        std::vector<T> data;
        std::vector<float> colors;
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

        void setColor(int color) {
            const float MAX_COLOR_VALUE = 255.0f;
            float red = (color & 0xFF) / MAX_COLOR_VALUE;
            float green = ((color >> 8) & 0xFF) / MAX_COLOR_VALUE;
            float blue = ((color >> 16) & 0xFF) / MAX_COLOR_VALUE;
            float alpha = ((color >> 24) & 0xFF) / MAX_COLOR_VALUE;

            auto vc = data.size() / this->dimension;
            for(auto i=0; i<vc; i++) {
                colors.push_back(red);
                colors.push_back(green);
                colors.push_back(blue);
                colors.push_back(alpha);
            }
        }

        void calculateVertexCount() {
            this->vertexCount = data.size() / this->dimension;
        }

        unsigned int getVertexCount() {
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

        unsigned int copyIndicesTo(unsigned short* anArray) {
            unsigned short* source = &indices[0];
            unsigned int size = indices.size();
            memcpy(anArray, source, size * sizeof(unsigned short));
            return size;
        }

        unsigned int copyColorTo(float* anArray) {
            float* source = &colors[0];
            unsigned int size = colors.size();
            memcpy(anArray, source, size * sizeof(float));
            return size;
        }

        T* getData() {
            return &data[0];
        }

        void addRow(int elementOnNewRow) {
            rowInfo.push_back(elementOnNewRow);
        }

        int getRowAt(int idx) { return rowInfo[idx]; }

        unsigned int getRowCount() { return rowInfo.size(); }

        void generateIndices();

        void setNormalOffset(int _normalOffset) {
            this->normalOffset = _normalOffset;
        }

        std::vector<short> getListIndices() const {
            return indices;
        }

        unsigned short* getIndices() {
            return &indices[0];
        }

        unsigned int indicesSize() const {
            return indices.size();
        }

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
    void ImageData<T>::generateIndices() {
        short count = 0;
        indices.clear();
        int rowCount = rowInfo.size();
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
                    
                        int e = count + vertexCountLine + 1;
                        int e1 = count + nexRowCount + 1;
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
    }
}

#endif

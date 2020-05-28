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
        ImageData(short dim);
        ImageData(unsigned int vcount, short dim, short noffs, const int* rows, int rowCount);
		~ImageData();

        T operator[] (int idx) {
            return data[idx];
        }

        void addData(T val) {
            data.push_back(val);
        }

        void addRow(int elementOnNewRow) {
            rowInfo.push_back(elementOnNewRow);
        }

        void generateIndices();

        std::vector<short> getIndices() const {
            return indices;
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

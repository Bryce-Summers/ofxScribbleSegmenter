#include "OffsetCurves.h"

namespace scrib
{

    OffsetCurves::OffsetCurves()
    {
        //ctor
    }

    OffsetCurves::~OffsetCurves()
    {
        //dtor
    }

    std::vector<scrib::point_info> * OffsetCurves::computeOffsetCurve(std::vector<ofPoint> * input, double dist)
    {
        cout << input->size() << endl;

        if (input->size() == 0)
        {
            std::vector<scrib::point_info> * trivial_output = new std::vector<scrib::point_info>();
            return trivial_output;
        }

        if (input->size() == 1)
        {
            std::vector<scrib::point_info> * circle_output = new std::vector<scrib::point_info>();

            cout << "size1" << endl;

            int len = 3;
            for (int i = 0; i < 3; i++)
            {
                circle_output->push_back(point_info(ofPoint(dist*cos(i / (PI * 2)), dist*sin(i / (PI * 2))), i));
            }


            return circle_output;
        }

        if (scrib::computeAreaOfPolygon(input) > 0)
        {
            dist *= -1;
        }

        std::vector<ofPoint> * unpruned_output;
        unpruned_output = offsetCurve(input, dist);
        std::vector<scrib::point_info> * pruned_output = computeExterior(unpruned_output);
        return pruned_output;
    }

    std::vector<ofPoint> * OffsetCurves::offsetCurve(std::vector<ofPoint> * input, double dist)
    {
        std::vector<ofPoint> perp_dirs;
        int len = input->size();
        for (int i = 0; i < len; i++)
        {
            int p0 = (i + len - 1) % len;
            int p1 = i;
            int p2 = (i + 1) % len;

            ofPoint point2 = input->at(p2);
            ofPoint point0 = input->at(p0);

            ofPoint tangent = point2 - point0;
            tangent.normalize();
            ofPoint perpendicular = ofVec2f(-tangent.y, tangent.x);
            perp_dirs.push_back(perpendicular);
        }

        std::vector<ofPoint> * output = new std::vector<ofPoint>();

        for (int i = 0; i < len; i++)
        {
            ofPoint original = input->at(i);
            ofPoint perp = perp_dirs[i];
            output->push_back(original + perp*dist);
        }

        return output;
    }

    // Prunes a curve of intermediary loops. Returns the curve consisting soley of its exterior.
    std::vector<scrib::point_info> * OffsetCurves::computeExterior(std::vector<ofPoint> * input)
    {
        scrib::FaceFinder segmenter;

        // Make sure the segmenter interprets the line as a closed loop.
        segmenter.setClosed(true);

        std::vector< std::vector<scrib::point_info> *> * faces;
        faces = segmenter.FindFaces(input);

        PolylineGraphPostProcessor post;
        post.load_face_vector(faces);

        std::vector<int> external_face_indices;
        post.determineComplementedFaces(&external_face_indices);

        // Get the index of the 1 external face.
        int output_index = external_face_indices[0];
        std::vector<scrib::point_info> * output = faces->at(output_index);

        // Deallocate superfluous face curves.
        int len = faces->size();
        for (int i = 0; i < len; i++)
        {
            if (i != output_index)
            {
                delete faces->at(i);
            }
        }

        delete faces;

        return output;
    }

}

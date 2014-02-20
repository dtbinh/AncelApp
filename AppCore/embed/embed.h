// Abstract embedding class.

#include <Eigen/Core>

using namespace Eigen;

class GPCMEmbed
{
protected:
    // Embedded points.
    MatrixXd &X;
    // Original points.
    MatrixXd &Y;
public:
    // Construct the embedding.
    GPCMEmbed(MatrixXd &X, MatrixXd &Y);
    // Virtual destructor.
    virtual ~GPCMEmbed();
};

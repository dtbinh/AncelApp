// Abstract embedding class.

#include "embed.h"

// Construct the embedding.
GPCMEmbed::GPCMEmbed(
    MatrixXd &X,                            // Embedded matrix.
    MatrixXd &Y                             // Design matrix.
    ) : X(X), Y(Y)
{
}

// Virtual destructor.
GPCMEmbed::~GPCMEmbed()
{
}

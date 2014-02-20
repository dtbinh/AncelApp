// Embed with PPCA.

#include "embed.h"

class GPCMEmbedPPCA : public GPCMEmbed
{
protected:
public:
    // Construct the embedding.
    GPCMEmbedPPCA(MatrixXd &X, MatrixXd &Y);
};

#include <QDebug>

#include "yefsmisc.h"
#include "yefsnode.h"
//==============================================================================================================================

void FsMisc::clearNodes(QHash<QString, FsNode *> &nodes)
{
	foreach (FsNode *node, nodes) {
		delete node;
	}

	nodes.clear();
}

void FsMisc::clearNodes(QList<FsNode *> &nodes)
{
	foreach (FsNode *node, nodes) {
		delete node;
	}

	nodes.clear();
}

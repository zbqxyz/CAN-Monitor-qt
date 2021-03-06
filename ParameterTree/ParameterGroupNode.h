#ifndef PARAMETERGROUPNODE_H
#define PARAMETERGROUPNODE_H

#include "ParameterTreeNode.h"

class ParameterGroupNode : public ParameterTreeNode
{
public:
    ParameterGroupNode(const QVariant &name="");
    bool acceptsChildren() const override;

    QVariant getData(parameterColumnFunction pcf, int role) const override;
    bool setData(parameterColumnFunction pcf, const QVariant &value) override;

    void writeDataToXml(QXmlStreamWriter &writer) const override;
    void readDataFromXml(QXmlStreamReader &reader) override;

    Qt::ItemFlags getFlags(parameterColumnFunction pcf, bool editMode) const override;
private:
    QVariant m_name;
};

#endif // PARAMETERGROUPNODE_H

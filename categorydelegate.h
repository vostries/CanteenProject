#ifndef CATEGORYDELEGATE_H
#define CATEGORYDELEGATE_H

#include <QStyledItemDelegate>
#include <QComboBox>

class CategoryDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    CategoryDelegate(QObject *parent = nullptr);
    
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;
};

#endif // CATEGORYDELEGATE_H
















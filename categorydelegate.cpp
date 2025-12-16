#include "categorydelegate.h"
#include "datamanager.h"
#include <QComboBox>

CategoryDelegate::CategoryDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {}

QWidget *CategoryDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const {
  Q_UNUSED(option);
  Q_UNUSED(index);

  QComboBox *combo = new QComboBox(parent);
  DataManager &dm = DataManager::getInstance();

  for (const Category &cat : dm.getCategories()) {
    combo->addItem(cat.getName(), cat.getId());
  }

  return combo;
}

void CategoryDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const {
  QComboBox *combo = qobject_cast<QComboBox *>(editor);
  if (!combo)
    return;

  int categoryId = index.data(Qt::UserRole).toInt();
  int comboIndex = combo->findData(categoryId);
  if (comboIndex >= 0) {
    combo->setCurrentIndex(comboIndex);
  }
}

void CategoryDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const {
  QComboBox *combo = qobject_cast<QComboBox *>(editor);
  if (!combo)
    return;

  QString categoryName = combo->currentText();
  int categoryId = combo->currentData().toInt();

  model->setData(index, categoryName, Qt::DisplayRole);
  model->setData(index, categoryId, Qt::UserRole);
}

void CategoryDelegate::updateEditorGeometry(QWidget *editor,
                                            const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const {
  Q_UNUSED(index);
  editor->setGeometry(option.rect);
}









#pragma once

#include <QFrame>
#include <QMap>

class Git;
class QStackedWidget;
class FullDiffWidget;
class FileDiffWidget;
class DiffButton;
class QVBoxLayout;

class DiffWidget : public QFrame
{
   Q_OBJECT

public:
   explicit DiffWidget(const QSharedPointer<Git> git, QWidget *parent = nullptr);

   void clear() const;
   void loadFileDiff(const QString &sha, const QString &previousSha, const QString &file);
   void loadCommitDiff(const QString &sha, const QString &parentSha);

private:
   QSharedPointer<Git> mGit;
   QStackedWidget *centerStackedWidget = nullptr;
   FullDiffWidget *mFullDiffWidget = nullptr;
   FileDiffWidget *mFileDiffWidget = nullptr;
   QMap<QString, QPair<QFrame *, DiffButton *>> mDiffButtons;
   QVBoxLayout *mDiffButtonsContainer = nullptr;
};

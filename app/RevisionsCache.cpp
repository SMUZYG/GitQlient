﻿#include "RevisionsCache.h"

RevisionsCache::RevisionsCache(QObject *parent)
   : QObject(parent)
{
}

void RevisionsCache::configure(int numElementsToStore)
{
   // We reserve 1 extra slots for the ZERO_SHA (aka WIP commit)
   mCommits.resize(numElementsToStore + 1);
   revs.reserve(numElementsToStore + 1);

   mCacheLocked = false;
}

CommitInfo RevisionsCache::getCommitInfoByRow(int row) const
{
   const auto commit = row >= 0 && row < mCommits.count() ? mCommits.at(row) : nullptr;

   return commit ? *commit : CommitInfo();
}

CommitInfo RevisionsCache::getCommitInfo(const QString &sha) const
{
   if (!sha.isEmpty())
   {
      CommitInfo *c;

      c = revs.value(sha, nullptr);

      if (c == nullptr)
      {
         const auto shas = revs.keys();

         const auto it = std::find_if(shas.cbegin(), shas.cend(),
                                      [sha](const QString &shaToCompare) { return shaToCompare.startsWith(sha); });

         if (it != shas.cend())
            return *revs.value(*it);
      }
      else
         return *c;
   }

   return CommitInfo();
}

void RevisionsCache::insertCommitInfo(CommitInfo rev)
{
   if (!mCacheLocked && !revs.contains(rev.sha()))
   {
      if (rev.lanes.count() == 0)
         updateLanes(rev, lns);

      const auto commit = new CommitInfo(rev);

      if (!mCommits[rev.orderIdx] || (mCommits[rev.orderIdx] && *commit != *mCommits[rev.orderIdx]))
      {
         if (mCommits[rev.orderIdx])
            delete mCommits[rev.orderIdx];

         mCommits[rev.orderIdx] = commit;
         revs.insert(rev.sha(), commit);
      }

      if (revs.contains(rev.parent(0)))
         revs.remove(rev.parent(0));
   }
}

void RevisionsCache::updateLanes(CommitInfo &c, Lanes &lns)
{
   const auto sha = c.sha();

   if (lns.isEmpty())
      lns.init(c.sha());

   bool isDiscontinuity;
   bool isFork = lns.isFork(sha, isDiscontinuity);
   bool isMerge = (c.parentsCount() > 1);
   bool isInitial = (c.parentsCount() == 0);

   if (isDiscontinuity)
      lns.changeActiveLane(sha); // uses previous isBoundary state

   lns.setBoundary(c.isBoundary()); // update must be here

   if (isFork)
      lns.setFork(sha);
   if (isMerge)
      lns.setMerge(c.parents());
   if (c.isApplied)
      lns.setApplied();
   if (isInitial)
      lns.setInitial();

   lns.setLanes(c.lanes); // here lanes are snapshotted

   const QString &nextSha = (isInitial) ? "" : QString(c.parent(0));

   lns.nextParent(nextSha);

   if (c.isApplied)
      lns.afterApplied();
   if (isMerge)
      lns.afterMerge();
   if (isFork)
      lns.afterFork();
   if (lns.isBranch())
      lns.afterBranch();
}

void RevisionsCache::clear()
{
   mCacheLocked = true;

   lns.clear();
   revs.clear();
}

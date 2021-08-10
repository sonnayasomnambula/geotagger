#include "selectionwatcher.h"

void SelectionWatcher::setCurrent(const QString& current)
{
    if (current != mCurrent) {
        mCurrent = current;
        emit currentChanged(mCurrent);
    }
}

#include "selectionwatcher.h"

void SelectionWatcher::setCurrent(int current)
{
    if (current != mCurrent) {
        mCurrent = current;
        emit currentChanged(mCurrent);
    }
}

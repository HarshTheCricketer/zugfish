# Zugfish UI refresh

## Changes in this update
- Reclaimed the space below the chessboard by moving the evaluation timeline into the Review panel.
- Increased the board's available vertical fit budget so it can grow while remaining square and keeping the player strips, evaluation bar, and navigation controls in view.
- Changed the review panel to align to the workspace's top and bottom padding and consume the remaining right-hand width.
- Reworked Mistake Recovery controls into a header row plus a wrapping action row. Return to review is now a visible secondary button without the old X icon; Best line and Punishment remain separate actions.
- Added a compact Game Momentum card with the existing interactive evaluation graph after review completes.
- Added subtle scale transitions and stronger hover borders to text and icon buttons.

## Functional scope
The changes are limited to QML layout and presentation. Existing controller actions and the game-review/training logic have not intentionally been changed.

## Validation
Source-level sanity checks were performed on the edited QML files (balanced braces and parentheses). A native Qt/QML build and interactive Windows regression test were not available in this packaging environment and should be performed after extraction.

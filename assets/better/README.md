# Better Character Assets

This directory contains the improved character animations for the player.

## Asset Source
Assets from: https://opengameart.org/content/animated-top-down-survivor-player

## Directory Structure

The animations should be placed in the following directories:

### Feet Animations
- `character/feet/idle/` - 1 frame (survivor-idle_0.png)
- `character/feet/walk/` - 20 frames (survivor-walk_0.png to survivor-walk_19.png)
- `character/feet/run/` - 20 frames (survivor-run_0.png to survivor-run_19.png)
- `character/feet/strafe_left/` - 20 frames (survivor-strafe_left_0.png to survivor-strafe_left_19.png)
- `character/feet/strafe_right/` - 20 frames (strafe_right_0.png to strafe_right_19.png)

### Handgun Animations
- `character/handgun/idle/` - 20 frames
- `character/handgun/move/` - 20 frames
- `character/handgun/shoot/` - 3 frames
- `character/handgun/reload/` - 15 frames
- `character/handgun/meleeattack/` - 15 frames

### Rifle Animations
- `character/rifle/idle/` - 20 frames
- `character/rifle/move/` - 20 frames
- `character/rifle/shoot/` - 3 frames
- `character/rifle/reload/` - 15 frames
- `character/rifle/meleeattack/` - 15 frames

### Shotgun Animations
Note: Currently using rifle animations as shotgun placeholder
- `character/shotgun/idle/` - 20 frames
- `character/shotgun/move/` - 20 frames
- `character/shotgun/shoot/` - 3 frames
- `character/shotgun/reload/` - 15 frames
- `character/shotgun/meleeattack/` - 15 frames

### Flashlight Animations
- `character/flashlight/idle/` - 20 frames
- `character/flashlight/move/` - 20 frames
- `character/flashlight/meleeattack/` - 15 frames

### Knife Animations
- `character/knife/idle/` - 20 frames
- `character/knife/move/` - 20 frames
- `character/knife/meleeattack/` - 15 frames

## File Naming Convention

Animation frames should be named with a numeric suffix indicating the frame number:
- `animationname_0.png`
- `animationname_1.png`
- `animationname_2.png`
- etc.

The code will automatically load all PNG files from each directory and sort them numerically.

## Integration

The code has been updated to:
1. Support separate feet and weapon animation layers
2. Handle multiple equipment types (knife, flashlight, handgun, rifle, shotgun)
3. Composite feet and weapon animations together during rendering
4. Use appropriate animation FPS settings for each animation type

Once the assets are downloaded and placed in these directories, the game will automatically use them.

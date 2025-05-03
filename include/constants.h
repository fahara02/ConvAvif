/**
 * @file constants.h
 * @brief Global constants used throughout the ConvAvif project
 * @author ConvAvif Developer
 * @date 2025
 * 
 * This file defines the key constant values used for image conversion
 * configuration, including RGB depth, quality ranges, and encoder-specific
 * parameter limits.
 */
#ifndef CONSTANTS_H
#define CONSTANTS_H

/**
 * @brief Bit depth for RGB image processing
 * 
 * Standard 8-bit per channel depth used for input/output RGB images
 */
static constexpr int RGB_DEPTH = 8;

/**
 * @brief Minimum encoder speed setting
 * 
 * Slowest speed setting (best quality, highest compression)
 */
static constexpr int MIN_SPEED = 0;

/**
 * @brief Maximum encoder speed setting
 * 
 * Fastest speed setting (lowest quality, fastest encoding)
 */
static constexpr int MAX_SPEED = 9;

/**
 * @brief Maximum quality value
 * 
 * Highest possible quality setting (99 out of 100)
 */
static constexpr int MAX_QUALITY = 99;

/**
 * @brief Maximum quantizer value
 * 
 * Highest quantizer value for the encoder (63)
 * Higher values mean lower quality but better compression
 */
static constexpr int MAX_QUANTIZER = 63;

/**
 * @brief Maximum tile log2 value
 * 
 * Controls maximum number of tiles as 2^MAX_TILE_LOG2
 */
static constexpr int MAX_TILE_LOG2 = 6;

/**
 * @brief Maximum sharpness filter value
 * 
 * Controls the strength of the sharpness filter (0-7)
 */
static constexpr int MAX_SHARPNESS = 7; // Sharpness range (0-7)

#endif
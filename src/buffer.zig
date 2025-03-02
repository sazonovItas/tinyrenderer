const std = @import("std");
const Vec2f = @import("math.zig").Vec2f;
const Vec2i = @import("math.zig").Vec2i;

const Error = error{
    ItemOutOfRange,
};

pub fn Buffer2D(comptime T: type) type {
    return struct {
        data: []T,
        width: usize,
        height: usize,

        const Self = @This();

        pub fn from(data: []T, width: usize) Self {
            return .{
                .data = data,
                .width = width,
                .height = @divExact(data.len, width),
            };
        }

        pub inline fn set(self: Self, x: i32, y: i32, item: T) void {
            if (x >= self.width or x < 0 or y >= self.height or y < 0) {
                return;
            }

            self.data[@as(usize, @intCast(x)) + self.width * @as(usize, @intCast(y))] = item;
        }

        pub inline fn get(self: Self, x: i32, y: i32) Error.ItemOutOfRange!T {
            if (x >= self.width or x < 0 or y >= self.height or y < 0) {
                return Error.ItemOutOfRange;
            }

            return self.data[@as(usize, @intCast(x)) + self.width * @as(usize, @intCast(y))];
        }

        pub fn at(self: Self, x: usize, y: usize) *T {
            return &self.data[x + self.width * y];
        }

        pub fn clear(self: Self, value: T) void {
            @memset(self.data, value);
        }
    };
}

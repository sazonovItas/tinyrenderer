const std = @import("std");
const Vec4f = @import("math.zig").Vec4f;
const M44 = @import("math.zig").M44;
const model = @import("model.zig");

pub const SimpleShader = struct {
    viewport: M44,
    proj_view_model: M44,

    const Self = @This();

    pub fn from(proj_view_model: M44) Self {
        return Self{ .proj_view_model = proj_view_model };
    }

    pub fn vertex(self: Self, mesh: model.Mesh, iobj: i32, index: i32) struct { vert: Vec4f, visible: bool } {
        const obj: model.Object = mesh.objects[iobj];
        const vertIdx: u32 = obj.indices[index].vertex;
        var vert: Vec4f = self.proj_view_model.apply_to_vec4(Vec4f.from(
            mesh.vertices[vertIdx][0],
            mesh.vertices[vertIdx][1],
            mesh.vertices[vertIdx][2],
            1.0,
        )).perspective_div();

        if (vert.w < 0.0) {
            return .{ .visible = false };
        }

        vert = self.viewport.apply_to_vec4(vert);

        return .{ .vert = vert, .visible = true };
    }
};
